#!/usr/bin/env python
"""Client/server correspondence diff.

Anchors server (a2serv6.exe, fully named by our migration) to retail client
(allods2.exe, mostly unnamed):

  1. RTTI vtable names      - only static MFC carries RTTI in these builds
  2. structural vtable match- vtables paired across programs by shared slot
                              function hashes (multiset); the server vtable's
                              class label = majority Ghidra namespace of its slots
  3. structural hash        - identical function bodies anywhere (tool/hash_funcs)
  4. name                   - client fns already named by the prior REA pass

Inputs (regenerate via tool/client_diff_collect.py + tool/hash_funcs.py on both
programs):
    build/client_diff_snapshot_a2serv6_exe.json   build/client_diff_snapshot_allods2_exe.json
    build/func_hashes_server.json                 build/func_hashes_client.json

Output:
    migration_state/client_diff.json         machine worklist
    migration_state/client_diff_report.md    per-class summary

Usage:
    python tool/client_diff.py
"""
import argparse
import glob
import json
import os
import re
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BUILD = os.path.join(ROOT, 'build')
STATE = os.path.join(ROOT, 'migration_state')

SRV_SNAP = os.path.join(BUILD, 'client_diff_snapshot_a2serv6_exe.json')
CLI_SNAP = os.path.join(BUILD, 'client_diff_snapshot_allods2_exe.json')
SRV_HASH = os.path.join(BUILD, 'func_hashes_server.json')
CLI_HASH = os.path.join(BUILD, 'func_hashes_client.json')

UNNAMED = re.compile(r'^(FUN_|sub_|thunk_|entry|AddrTable|switchD|nullsub|j_|unknown_libname|loc_)')


def rtti_class(raw):
    if not raw or not raw.startswith('.?A'):
        return None
    return raw[4:].rstrip('@').split('@')[0]


def real(name):
    return not UNNAMED.match(name)


def load_hashes(path):
    return {int(it['entry'], 16): it for it in json.load(open(path))}


def majority_ns(slot_vas, funcs):
    votes = Counter()
    for va in slot_vas:
        f = funcs.get(va)
        if f and f['ns']:
            votes[f['ns'][-1]] += 1
    if not votes:
        return None
    top, n = votes.most_common(1)[0]
    return top if n * 2 > sum(votes.values()) or len(votes) == 1 else None


def vtable_hashes(slots, hashes):
    return [hashes[va]['h_shape'] for va in slots if va in hashes]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--min-shared', type=int, default=3,
                    help='min shared slot hashes to pair two vtables')
    ap.add_argument('--min-ratio', type=float, default=0.34,
                    help='shared/min(len) ratio to pair two vtables')
    a = ap.parse_args()

    srv_snap = json.load(open(SRV_SNAP))
    cli_snap = json.load(open(CLI_SNAP))
    srv_funcs = {f['va']: f for f in srv_snap['functions']}
    cli_funcs = {f['va']: f for f in cli_snap['functions']}
    srv_h = load_hashes(SRV_HASH)
    cli_h = load_hashes(CLI_HASH)
    srv_by_hash = defaultdict(list)
    for va, h in srv_h.items():
        srv_by_hash[h['h_shape']].append(va)
    cli_hash_map = {}
    for va, h in cli_h.items():
        cli_hash_map.setdefault(h['h_shape'], va)

    # --- label vtables ------------------------------------------------------
    for vt in srv_snap['vtables']:
        vt['label'] = majority_ns(vt['slots'], srv_funcs) or rtti_class(vt.get('rtti'))
    for vt in cli_snap['vtables']:
        vt['label'] = majority_ns(vt['slots'], cli_funcs) or rtti_class(vt.get('rtti'))

    # --- match vtables across programs ---------------------------------------
    srv_by_rtti = {vt['label']: vt for vt in srv_snap['vtables']
                   if vt.get('rtti') and vt['label']}
    pairs_vt = []  # (srv_vt, cli_vt, basis)
    used_srv_vt = set()
    used_cli_vt = set()
    # pass 1: RTTI name (MFC classes)
    for vt in cli_snap['vtables']:
        if vt.get('rtti') and vt['label'] in srv_by_rtti:
            pairs_vt.append((srv_by_rtti[vt['label']], vt, 'rtti'))
            used_srv_vt.add(srv_by_rtti[vt['label']]['va'])
            used_cli_vt.add(vt['va'])
    # pass 2: structural (multiset of slot hashes)
    cli_free = [vt for vt in cli_snap['vtables'] if vt['va'] not in used_cli_vt]
    srv_free = [vt for vt in srv_snap['vtables'] if vt['va'] not in used_srv_vt]
    cand = []
    for sv in srv_free:
        sh = Counter(vtable_hashes(sv['slots'], srv_h))
        if not sh:
            continue
        for cv in cli_free:
            ch = Counter(vtable_hashes(cv['slots'], cli_h))
            if not ch:
                continue
            shared = sum((sh & ch).values())
            if shared >= a.min_shared and shared >= a.min_ratio * min(len(sv['slots']), len(cv['slots'])):
                cand.append((shared, sv, cv))
    cand.sort(key=lambda x: -x[0])
    for shared, sv, cv in cand:
        if sv['va'] in used_srv_vt or cv['va'] in used_cli_vt:
            continue
        pairs_vt.append((sv, cv, 'struct:%d' % shared))
        used_srv_vt.add(sv['va'])
        used_cli_vt.add(cv['va'])

    # --- class universe ------------------------------------------------------
    classes = {}
    for vt in srv_snap['vtables']:
        lbl = vt['label'] or 'vt@%06x' % vt['va']
        classes.setdefault(lbl, {'srv_members': {}, 'cli_members': {}, 'vt_pairs': []})
        for s in vt['slots']:
            classes[lbl]['srv_members'].setdefault(s, 'vt')
    for va, f in srv_funcs.items():
        if f['ns']:
            classes.setdefault(f['ns'][-1], {'srv_members': {}, 'cli_members': {}, 'vt_pairs': []})
            classes[f['ns'][-1]]['srv_members'].setdefault(va, 'ns')
    for sv, cv, basis in pairs_vt:
        lbl = sv['label'] or 'vt@%06x' % sv['va']
        classes.setdefault(lbl, {'srv_members': {}, 'cli_members': {}, 'vt_pairs': []})
        for s in sv['slots']:
            classes[lbl]['srv_members'].setdefault(s, 'vt')
        for s in cv['slots']:
            classes[lbl]['cli_members'].setdefault(s, 'vt')
        classes[lbl]['vt_pairs'].append((sv, cv))
    for va, f in cli_funcs.items():
        if f['ns']:
            classes.setdefault(f['ns'][-1], {'srv_members': {}, 'cli_members': {}, 'vt_pairs': []})
            classes[f['ns'][-1]]['cli_members'].setdefault(va, 'ns')
    for vt in cli_snap['vtables']:
        if vt['va'] not in used_cli_vt and vt['label'] and vt['label'] not in classes:
            classes.setdefault('cli:' + str(vt['label']), {'srv_members': {}, 'cli_members': {}, 'vt_pairs': []})
            for s in vt['slots']:
                classes['cli:' + str(vt['label'])]['cli_members'].setdefault(s, 'vt')

    # --- repo class -> header map -------------------------------------------
    repo = defaultdict(set)
    pat = re.compile(r'^\s*(?:class|struct)\s+([A-Za-z_]\w*)')
    for path in glob.glob(os.path.join(ROOT, 'src', '*.h')):
        for ln in open(path, encoding='utf-8', errors='replace'):
            m = pat.match(ln)
            if m:
                repo[m.group(1)].add(os.path.basename(path))

    # --- pair methods per class ----------------------------------------------
    worklist = []
    stats = Counter()
    report = []

    for cname, rec in classes.items():
        srv_members = rec['srv_members']
        cli_members = rec['cli_members']
        pairs = []
        used_srv, used_cli = set(), set()

        # 1. hash pairs (class-scoped)
        for cva in list(cli_members):
            h = cli_h.get(cva)
            if not h:
                continue
            for sva in srv_by_hash.get(h['h_shape'], []):
                if sva in srv_members:
                    pairs.append((sva, cva, 'IDENTICAL', 'hash'))
                    used_srv.add(sva)
                    used_cli.add(cva)
                    break

        # 2. name pairs (one-to-one; on collisions pick closest body size)
        cli_named = defaultdict(list)
        for v in cli_members:
            if v not in used_cli and v in cli_funcs and real(cli_funcs[v]['name']):
                cli_named[cli_funcs[v]['name']].append(v)
        for sva in sorted(srv_members):
            if sva in used_srv or sva not in srv_funcs or not real(srv_funcs[sva]['name']):
                continue
            cands = [v for v in cli_named.get(srv_funcs[sva]['name'], []) if v not in used_cli]
            if not cands:
                continue
            s_sz = max(srv_funcs[sva]['size'], 1)
            cva = min(cands, key=lambda v: abs((cli_funcs[v]['size'] or 1) - s_sz) / s_sz)
            pairs.append((sva, cva, 'DIFFERS', 'name'))
            used_srv.add(sva)
            used_cli.add(cva)

        # 3. positional pairs inside each matched vtable pair of THIS class,
        #    offset anchored on hash-equal slot positions
        for sv, cv in rec.get('vt_pairs', []):
            ss, cs = sv['slots'], cv['slots']
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
                if sva in used_srv or cva in used_cli:
                    continue
                if sva not in srv_members or cva not in cli_members:
                    continue
                pairs.append((sva, cva, 'DIFFERS', 'slot%d' % i))
                used_srv.add(sva)
                used_cli.add(cva)

        for sva, cva, status, basis in pairs:
            s_sz = srv_funcs[sva]['size'] if sva in srv_funcs else 0
            c_sz = cli_funcs[cva]['size'] if cva in cli_funcs else 0
            worklist.append({
                'class': cname, 'method': srv_funcs[sva]['name'] if sva in srv_funcs else '?',
                'status': status, 'basis': basis,
                'slot': basis[4:] if basis.startswith('slot') else None,
                'server_va': '%06x' % sva, 'client_va': '%06x' % cva,
                'server_size': s_sz, 'client_size': c_sz,
                'server_thunk': srv_funcs[sva]['thunk'] if sva in srv_funcs else False,
            })
            stats[status] += 1
        for sva in sorted(set(srv_members) - used_srv):
            # identical twin anywhere in the client? then not really server-only
            h = srv_h.get(sva)
            twin = cli_hash_map.get(h['h_shape']) if h else None
            if twin is not None:
                worklist.append({'class': cname, 'method': srv_funcs[sva]['name'],
                                 'status': 'IDENTICAL', 'basis': 'global-hash', 'slot': None,
                                 'server_va': '%06x' % sva, 'client_va': '%06x' % twin,
                                 'server_size': srv_funcs[sva]['size'],
                                 'client_size': cli_funcs.get(twin, {}).get('size', 0),
                                 'server_thunk': srv_funcs[sva]['thunk']})
                stats['IDENTICAL'] += 1
                continue
            worklist.append({'class': cname, 'method': srv_funcs[sva]['name'],
                             'status': 'SERVER_ONLY', 'basis': srv_members[sva],
                             'slot': None, 'server_va': '%06x' % sva, 'client_va': None,
                             'server_size': srv_funcs[sva]['size'], 'client_size': 0,
                             'server_thunk': srv_funcs[sva]['thunk']})
            stats['SERVER_ONLY'] += 1
        for cva in sorted(set(cli_members) - used_cli):
            h = cli_h.get(cva)
            twin = srv_by_hash.get(h['h_shape'], [None])[0] if h else None
            if twin is not None:
                worklist.append({'class': cname,
                                 'method': cli_funcs[cva]['name'] if cva in cli_funcs else '?',
                                 'status': 'IDENTICAL', 'basis': 'global-hash', 'slot': None,
                                 'server_va': '%06x' % twin, 'client_va': '%06x' % cva,
                                 'server_size': srv_funcs.get(twin, {}).get('size', 0),
                                 'client_size': cli_funcs[cva]['size'] if cva in cli_funcs else 0,
                                 'server_thunk': False})
                stats['IDENTICAL'] += 1
                continue
            worklist.append({'class': cname,
                             'method': cli_funcs[cva]['name'] if cva in cli_funcs else '?',
                             'status': 'CLIENT_ONLY', 'basis': cli_members[cva],
                             'slot': None, 'server_va': None, 'client_va': '%06x' % cva,
                             'server_size': 0,
                             'client_size': cli_funcs[cva]['size'] if cva in cli_funcs else 0,
                             'server_thunk': False})
            stats['CLIENT_ONLY'] += 1

        n_id = sum(1 for p in pairs if p[2] == 'IDENTICAL')
        report.append({'class': cname, 'in_repo': sorted(repo.get(cname.lstrip('cli:'), []))})

    # --- cross-tier name pass: server methods recorded SERVER_ONLY whose client
    #     twin sits unrecognized in the free tier (non-virtual, e.g. ProcessPackets)
    paired_cli_vas = {int(e['client_va'], 16) for e in worklist if e['client_va']}
    srv_by_name = defaultdict(list)
    for e in worklist:
        if e['status'] == 'SERVER_ONLY' and real(e['method']):
            srv_by_name[e['method']].append(e)
    cli_free_named = defaultdict(list)
    for va, f in cli_funcs.items():
        if va in paired_cli_vas or not real(f['name']):
            continue
        h = cli_h.get(va)
        if h and h['h_shape'] in srv_by_hash:
            continue  # identical twin exists; not a DIFFERS candidate
        cli_free_named[f['name']].append(va)
    for name, entries in srv_by_name.items():
        cands = cli_free_named.get(name)
        if not cands:
            continue
        entries = sorted(entries, key=lambda e: -e['server_size'])
        for e in entries[:len(cands)]:
            s_sz = max(e['server_size'], 1)
            cva = min(cands, key=lambda v: abs((cli_funcs[v]['size'] or 1) - s_sz) / s_sz)
            cands.remove(cva)
            e['status'] = 'DIFFERS'
            e['basis'] = 'xname'
            e['client_va'] = '%06x' % cva
            e['client_size'] = cli_funcs[cva]['size']

    # per-class tallies computed from the final worklist
    tally = defaultdict(lambda: Counter())
    for e in worklist:
        t = tally[e['class']]
        t[e['status']] += 1
        if e['status'] == 'DIFFERS':
            t['diff_bytes'] += e['client_size']
        if e['status'] == 'CLIENT_ONLY':
            t['new_bytes'] += e['client_size']
    for r in report:
        t = tally[r['class']]
        r['identical'] = t['IDENTICAL']
        r['differs'] = t['DIFFERS']
        r['server_only'] = t['SERVER_ONLY']
        r['client_only'] = t['CLIENT_ONLY']
        r['client_diff_bytes'] = t['diff_bytes']
        r['client_new_bytes'] = t['new_bytes']

    # --- free-function tier ---------------------------------------------------
    class_cli_vas = {int(e['client_va'], 16) for e in worklist if e['client_va']}
    free_id = free_un = free_un_bytes = 0
    for va, h in cli_h.items():
        if va in class_cli_vas:
            continue
        if h['h_shape'] in srv_by_hash:
            free_id += 1
        else:
            free_un += 1
            free_un_bytes += h['size']

    # --- output ---------------------------------------------------------------
    report.sort(key=lambda r: -(r['client_diff_bytes'] + r['client_new_bytes']))
    os.makedirs(STATE, exist_ok=True)
    with open(os.path.join(STATE, 'client_diff.json'), 'w') as fh:
        json.dump({'classes': report, 'worklist': worklist,
                   'vtable_pairs': [{'server': '%06x' % sv['va'], 'client': '%06x' % cv['va'],
                                     'class': sv['label'], 'basis': b} for sv, cv, b in pairs_vt],
                   'free_functions': {'identical': free_id, 'no_twin': free_un,
                                      'no_twin_bytes': free_un_bytes}}, fh, indent=1)

    lines = ['# Client/server class-level diff (auto-generated by tool/client_diff.py)', '',
             '| class | repo header | identical | differs | srv-only | cli-only | cli diff KB | cli new KB |',
             '|---|---|---|---|---|---|---|---|']
    for r in report:
        if r['identical'] + r['differs'] + r['server_only'] + r['client_only'] == 0:
            continue
        lines.append('| {c} | {r} | {i} | {d} | {so} | {co} | {db:.0f} | {nb:.0f} |'.format(
            c=r['class'], r=','.join(r['in_repo']) or '-',
            i=r['identical'], d=r['differs'], so=r['server_only'], co=r['client_only'],
            db=r['client_diff_bytes'] / 1024, nb=r['client_new_bytes'] / 1024))
    lines.append('')
    lines.append('Free functions (no class anchor): %d identical, %d without server twin (%.0f KB).'
                 % (free_id, free_un, free_un_bytes / 1024))
    with open(os.path.join(STATE, 'client_diff_report.md'), 'w') as fh:
        fh.write('\n'.join(lines) + '\n')

    print('vtable pairs: %d (rtti %d, structural %d)' % (
        len(pairs_vt), sum(1 for _, _, b in pairs_vt if b == 'rtti'),
        sum(1 for _, _, b in pairs_vt if b.startswith('struct'))))
    print('classes:', len(report))
    final_stats = Counter(e['status'] for e in worklist)
    print('method statuses:', dict(final_stats))
    print('free functions: %d identical, %d no-twin (%.0f KB)' % (free_id, free_un, free_un_bytes / 1024))
    print('wrote migration_state/client_diff.json + client_diff_report.md')
    print()
    print('top classes by client workload:')
    for r in report[:20]:
        print('  %-26s %-12s id=%-4d diff=%-3d srv-only=%-4d cli-only=%-3d %6.0f KB diff + %5.0f KB new' % (
            r['class'][:26], (','.join(r['in_repo']) or '-')[:12], r['identical'], r['differs'],
            r['server_only'], r['client_only'],
            r['client_diff_bytes'] / 1024, r['client_new_bytes'] / 1024))


if __name__ == '__main__':
    main()
