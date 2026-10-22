#!/usr/bin/env python
"""Per-class client-vs-server method comparison.

    python tool/client_fn_diff.py VisShop

Prints every method of the class with status:
  IDENTICAL   hash-identical twin in client - current C++ is already client-correct
  DIFFERS     paired (vtable slot / call-site / name) but bodies differ - needs client port
  SERVER_ONLY no client counterpart found (server-newer, or pairing gap)
  CLIENT_ONLY client method without server counterpart

`class_table(cls)` is importable (used by tool/client_verify.py).
"""
import argparse
import json
import os
import re
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BUILD = os.path.join(ROOT, 'build')
STATE = os.path.join(ROOT, 'migration_state')

UNNAMED = re.compile(r'^(FUN_|sub_|thunk_|entry|AddrTable|switchD|nullsub_|j_|unknown_libname|loc_)')

_AR = None


# Cross-program namespace aliases: the same class can carry different primary
# Ghidra namespaces in the two programs (client names the frame window
# CMainWindow, the server MainWindow). Auto-derivation from hash-identical
# twins was tried and is untrustworthy - identical machine code is common
# across UNRELATED classes (MFC thunks, template bodies, import stubs), so the
# votes scatter. This table is hand-verified; extend it when a class shows a
# false SERVER_ONLY + CLIENT_ONLY split with matching vtable evidence.
NS_ALIAS = {
    'CMainWindow': 'MainWindow',
    'CMap': 'CMap<>',
    'VisCharGenTextBox': 'VisStartGameTextBox',
    'GO_d0': 'TokenEntry',
    'COleDataObject': 'FieldBlock5859c',
    'ResFile': 'ResBase',
}


def artifacts():
    global _AR
    if _AR is not None:
        return _AR
    cm = json.load(open(os.path.join(STATE, 'client_classmap.json')))
    srv = json.load(open(os.path.join(BUILD, 'client_diff_snapshot_a2serv6_exe.json')))
    cli = json.load(open(os.path.join(BUILD, 'client_diff_snapshot_allods2_exe.json')))
    srv_h = {int(it['entry'], 16): it for it in json.load(open(os.path.join(BUILD, 'func_hashes_server.json')))}
    cli_h = {int(it['entry'], 16): it for it in json.load(open(os.path.join(BUILD, 'func_hashes_client.json')))}
    srv_funcs = {f['va']: f for f in srv['functions']}
    cli_funcs = {f['va']: f for f in cli['functions']}

    def refs_of(snap):
        r = defaultdict(list)
        for fn, tos in snap['datarefs'].items():
            for to in tos:
                r[to].append(int(fn, 16))
        return r

    def split(runs, refs_to):
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
                if end - c >= 2:
                    pieces[va + c * 4] = slots[c:end]
        return pieces

    srv_pieces = split(srv['vtables'], refs_of(srv))
    cli_pieces = split(cli['vtables'], refs_of(cli))
    cli_shape_map = defaultdict(list)
    for c, h in cli_h.items():
        cli_shape_map[h['h_shape']].append(c)
    srv_shape_map = defaultdict(list)
    for s, h in srv_h.items():
        srv_shape_map[h['h_shape']].append(s)

    _AR = {
        'cm': cm, 'srv': srv, 'cli': cli, 'srv_h': srv_h, 'cli_h': cli_h,
        'srv_funcs': srv_funcs, 'cli_funcs': cli_funcs,
        'srv_pieces': srv_pieces, 'cli_pieces': cli_pieces,
        'cli_shape_map': cli_shape_map, 'srv_shape_map': srv_shape_map,
        'ns_alias': dict(NS_ALIAS),
    }
    return _AR


def class_table(cls):
    """-> dict(header=..., rows=[{status, method, basis, server_va, client_va,
    server_size, client_size}, ...])"""
    ar = artifacts()
    cm, srv_funcs, cli_funcs = ar['cm'], ar['srv_funcs'], ar['cli_funcs']
    srv_h, cli_h = ar['srv_h'], ar['cli_h']
    rec = cm['classes'].get(cls)
    if rec is None:
        return None

    cre = re.compile(r'_conflict\d+$')
    ns_alias = ar['ns_alias']
    srv_members = {va for va, f in srv_funcs.items()
                   if f['ns'] and cre.sub('', f['ns'][-1]) == cls}
    for va, b, n in rec['srv_vts']:
        srv_members.update(ar['srv_pieces'].get(int(va, 16), []))
    cli_members = set()
    # client namespace members (e.g. CMainWindow::) join via the alias votes;
    # Ghidra's _conflictN suffix is a naming artifact and always collapses
    for va, f in cli_funcs.items():
        if f['ns']:
            cn = cre.sub('', f['ns'][-1])
            if cn == cls or ns_alias.get(cn) == cls:
                cli_members.add(va)
    for va, b, n in rec['cli_vts']:
        cli_members.update(ar['cli_pieces'].get(int(va, 16), []))
    if rec['ctor_cli']:
        cli_members.add(int(rec['ctor_cli'], 16))
    for e in cm['callsite_pairs']:
        if e['server_class'] == cls:
            cli_members.add(int(e['client'], 16))
            srv_members.add(int(e['server'], 16))

    used_s, used_c = set(), set()
    rows = []

    def add(status, basis, sva, cva):
        rows.append({'status': status, 'method': (srv_funcs.get(sva) or cli_funcs.get(cva) or {}).get('name', '?'),
                     'basis': basis,
                     'server_va': '%06x' % sva if sva else None,
                     'client_va': '%06x' % cva if cva else None,
                     'server_size': srv_funcs.get(sva, {}).get('size', 0),
                     'client_size': cli_funcs.get(cva, {}).get('size', 0)})

    # 1. hash-identical (class-scoped, server va used once)
    for cva in sorted(cli_members):
        h = cli_h.get(cva)
        if not h:
            continue
        twins = [s for s in srv_members if s not in used_s and s in srv_h
                 and srv_h[s]['h_shape'] == h['h_shape']]
        if twins:
            add('IDENTICAL', 'hash', twins[0], cva)
            used_s.add(twins[0])
            used_c.add(cva)

    # 2. vtable slot alignment
    for sva_rec, cva_rec in zip(rec['srv_vts'], rec['cli_vts']):
        ss = ar['srv_pieces'].get(int(sva_rec[0], 16), [])
        cs = ar['cli_pieces'].get(int(cva_rec[0], 16), [])
        best_d, best_n = 0, -1
        for d in range(-6, 7):
            n = 0
            for i in range(len(ss)):
                j = i + d
                if 0 <= j < len(cs):
                    h1 = srv_h.get(ss[i], {}).get('h_shape')
                    h2 = cli_h.get(cs[j], {}).get('h_shape')
                    if h1 and h1 == h2:
                        n += 1
            if n > best_n:
                best_n, best_d = n, d
        for i in range(len(ss)):
            j = i + best_d
            if not (0 <= j < len(cs)):
                continue
            sva, cva = ss[i], cs[j]
            if sva in used_s or cva in used_c:
                continue
            same = srv_h.get(sva, {}).get('h_shape') == cli_h.get(cva, {}).get('h_shape')
            add('IDENTICAL' if same else 'DIFFERS', 'slot%d' % i, sva, cva)
            used_s.add(sva)
            used_c.add(cva)

    # 3. call-site pairs from the classmap (identical callers, aligned call sites)
    for e in cm['callsite_pairs']:
        sva = int(e['server'], 16)
        cva = int(e['client'], 16)
        if sva in srv_members and cva in cli_members and sva not in used_s and cva not in used_c:
            add('DIFFERS', 'callsite:%d votes' % e['votes'], sva, cva)
            used_s.add(sva)
            used_c.add(cva)

    # 4. names (server members real-named, client members from REA/pushed names;
    #    bare identifier only - client names may carry a '(params)' signature suffix)
    cli_named = defaultdict(list)
    for v in cli_members - used_c:
        f = cli_funcs.get(v)
        if f and f['name'] and not UNNAMED.match(f['name']):
            cli_named.setdefault(f['name'].split('(')[0], []).append(v)
    for sva in sorted(srv_members - used_s):
        f = srv_funcs.get(sva)
        if not f or UNNAMED.match(f['name']):
            continue
        cands = [v for v in cli_named.get(f['name'].split('(')[0], []) if v not in used_c]
        if cands:
            s_sz = max(f['size'], 1)
            cva = min(cands, key=lambda v: abs((cli_funcs[v]['size'] or 1) - s_sz) / s_sz)
            add('DIFFERS', 'name', sva, cva)
            used_s.add(sva)
            used_c.add(cva)

    for sva in sorted(srv_members - used_s):
        h = srv_h.get(sva)
        twin = ar['cli_shape_map'].get(h['h_shape'], [None])[0] if h else None
        if twin is not None:
            add('IDENTICAL', 'global-hash', sva, twin)
            used_c.discard(twin)
            continue
        add('SERVER_ONLY', 'ns/vt', sva, None)
    for cva in sorted(cli_members - used_c):
        add('CLIENT_ONLY', 'vt', None, cva)

    order = {'IDENTICAL': 0, 'DIFFERS': 1, 'CLIENT_ONLY': 2, 'SERVER_ONLY': 3}
    rows.sort(key=lambda r: (order[r['status']], -r['server_size']))
    header = 'class %s  bases=%s  ctor %s -> %s' % (
        cls, ','.join(rec['bases']) or '-', rec['ctor_srv'] or '-', rec['ctor_cli'] or '-')
    return {'header': header, 'rows': rows}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('cls')
    a = ap.parse_args()
    t = class_table(a.cls)
    if t is None:
        print('class not in map:', a.cls)
        return
    print(t['header'])
    print()
    print('%-12s %-28s %-9s %-9s %-7s %-7s %s' % ('status', 'method', 'srv', 'cli', 'srv_sz', 'cli_sz', 'basis'))
    for r in t['rows']:
        print('%-12s %-28s %-9s %-9s %-7d %-7d %s' % (
            r['status'], r['method'][:28], r['server_va'] or '-', r['client_va'] or '-',
            r['server_size'], r['client_size'], r['basis']))
    from collections import Counter
    print()
    print(dict(Counter(r['status'] for r in t['rows'])))


if __name__ == '__main__':
    main()
