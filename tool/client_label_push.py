# Runs INSIDE the Ghidra daemon on the CLIENT program (allods2.exe):
#   MSYS_NO_PATHCONV=1 python tool/ghidra_daemon.py --program /allods2.exe
#   python tool/gh.py python -f tool/client_label_push.py
#
# Pushes the confirmed class map into the client Ghidra program:
#   - client vtables get a '<Class>_vtable' label (placeholder names only)
#   - matched vtable slots: client fn -> server fn's class namespace + name
#   - client ctors of matched classes -> <Class>::<Class>
#   - call-site-paired client fns -> server fn's namespace + name
# Placeholder client names only (FUN_*/sub_*/thunk_*...) - real names win.
# Does NOT save; run `python tool/gh.py save` after reviewing the summary.

import json
import re
from collections import Counter, defaultdict

from ghidra.program.model.symbol import SourceType

MAP = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\migration_state\client_classmap.json'
SRV_SNAP = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\client_diff_snapshot_a2serv6_exe.json'
CLI_SNAP = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\client_diff_snapshot_allods2_exe.json'
SRV_HASH = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\func_hashes_server.json'
CLI_HASH = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\func_hashes_client.json'

PLACEHOLDER = re.compile(r'^(FUN_|SUB_|sub_|thunk_|j_|nullsub_|unknown_libname|loc_|\?)')
BAD_NS = re.compile(r'[<>,\s]')

m = json.load(open(MAP))
srv_snap = json.load(open(SRV_SNAP))
cli_snap = json.load(open(CLI_SNAP))
srv_h = {int(it['entry'], 16): it for it in json.load(open(SRV_HASH))}
cli_h = {int(it['entry'], 16): it for it in json.load(open(CLI_HASH))}
srv_funcs = {f['va']: f for f in srv_snap['functions']}
cli_funcs = {f['va']: f for f in cli_snap['functions']}
st = program.getSymbolTable()
fm = program.getFunctionManager()

# rebuild pieces exactly like tool/client_classmap.py


def refs_of(snap):
    r = defaultdict(list)
    for fn, tos in snap['datarefs'].items():
        fn = int(fn, 16)
        for to in tos:
            r[to].append(fn)
    return r


def split_runs(runs, refs_to):
    pieces = {}
    for run in runs:
        va, slots = run['va'], run['slots']
        cuts = {0}
        for off in range(4, len(slots) * 4, 4):
            if (va + off) in refs_to:
                cuts.add(off // 4)
        cuts = sorted(cuts)
        for i, c in enumerate(cuts):
            end = cuts[i + 1] if i + 1 < len(cuts) else len(slots)
            if end - c < 2:
                continue
            pieces[va + c * 4] = slots[c:end]
    return pieces


srv_pieces = split_runs(srv_snap['vtables'], refs_of(srv_snap))
cli_pieces = split_runs(cli_snap['vtables'], refs_of(cli_snap))


def anchored_pairs(sslots, cslots):
    """positional slot pairs offset-anchored on hash-equal positions"""
    best_d, best_n = 0, -1
    for d in range(-6, 7):
        n = 0
        for i in range(len(sslots)):
            j = i + d
            if 0 <= j < len(cslots):
                h1 = srv_h.get(sslots[i], {}).get('h_shape')
                h2 = cli_h.get(cslots[j], {}).get('h_shape')
                if h1 and h1 == h2:
                    n += 1
        if n > best_n:
            best_n, best_d = n, d
    out = []
    for i in range(len(sslots)):
        j = i + best_d
        if 0 <= j < len(cslots):
            out.append((sslots[i], cslots[j]))
    return out


renamed = Counter()
skipped_real = Counter()
skipped_dup = 0
done = set()          # client vas already renamed by this push
used_in_ns = defaultdict(set)  # (ns tuple, name) already assigned


def valid_ns(f):
    return f['ns'] and not any(BAD_NS.search(p) for p in f['ns'])


def rename_fn(cva, sf):
    """rename client fn at cva to server fn sf's namespace+name"""
    global skipped_dup
    if cva in done:
        skipped_dup += 1
        return
    cf = cli_funcs.get(cva)
    if cf is None:
        return
    if not PLACEHOLDER.match(cf['name']):
        skipped_real[cf['name']] += 1
        return
    if not valid_ns(sf):
        return
    key = (tuple(sf['ns']), sf['name'])
    if sf['name'] in used_in_ns[tuple(sf['ns'])]:
        skipped_dup += 1
        return
    f = fm.getFunctionAt(program.getAddressFactory().getAddress(hex(cva)))
    if f is None:
        return
    ns = ctx.namespace(list(sf['ns']))
    f.getSymbol().setNameAndNamespace(sf['name'], ns, SourceType.USER_DEFINED)
    used_in_ns[tuple(sf['ns'])].add(sf['name'])
    done.add(cva)
    renamed['slots+%s' % sf['ns'][-1]] += 1


with ctx.transaction('client label push'):
    # 1. vtable data labels
    for vp in m['vtable_pairs']:
        cls = vp['class']
        if not cls or BAD_NS.search(cls):
            continue
        cva = int(vp['client_vt'], 16)
        addr = program.getAddressFactory().getAddress(hex(cva))
        sym = st.getPrimarySymbol(addr)
        name = cls + '_vtable'
        if sym is not None:
            if PLACEHOLDER.match(sym.getName()) or sym.getName().startswith('off_'):
                sym.setName(name, SourceType.USER_DEFINED)
                renamed['vtable_labels'] += 1
            else:
                skipped_real[sym.getName()] += 1
        else:
            st.createLabel(addr, name, SourceType.USER_DEFINED)
            renamed['vtable_labels'] += 1

    # 2. vtable slot function names (server slot -> client slot)
    for vp in m['vtable_pairs']:
        sv, cv = int(vp['server_vt'], 16), int(vp['client_vt'], 16)
        if sv not in srv_pieces or cv not in cli_pieces:
            continue
        for s_slot, c_slot in anchored_pairs(srv_pieces[sv], cli_pieces[cv]):
            sf = srv_funcs.get(s_slot)
            if sf is None or sf['thunk']:
                continue
            if not sf['ns']:
                continue
            rename_fn(c_slot, sf)

    # 3. ctors of matched classes
    for cls, rec in m['classes'].items():
        if not rec.get('ctor_cli') or not rec.get('ctor_srv'):
            continue
        sf = srv_funcs.get(int(rec['ctor_srv'], 16))
        if sf is None or not valid_ns(sf):
            continue
        cva = int(rec['ctor_cli'], 16)
        cf = cli_funcs.get(cva)
        if cf is None or not PLACEHOLDER.match(cf['name']):
            continue
        if cva in done:
            continue
        f = fm.getFunctionAt(program.getAddressFactory().getAddress(hex(cva)))
        if f is None:
            continue
        ns = ctx.namespace(list(sf['ns']))
        f.getSymbol().setNameAndNamespace(sf['name'], ns, SourceType.USER_DEFINED)
        done.add(cva)
        used_in_ns[tuple(sf['ns'])].add(sf['name'])
        renamed['ctors'] += 1

    # 4. call-site pairs
    for e in m['callsite_pairs']:
        sf = srv_funcs.get(int(e['server'], 16))
        if sf is None:
            continue
        rename_fn(int(e['client'], 16), sf)

print('renamed:', dict(renamed))
print('total fns renamed:', len(done))
print('skipped (real client name kept):', sum(skipped_real.values()), 'distinct:', len(skipped_real))
print('skipped (dup/ns-invalid):', skipped_dup)
print('NOT SAVED - run: python tool/gh.py save')
