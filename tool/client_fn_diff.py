#!/usr/bin/env python
"""Per-class client-vs-server method comparison.

    python tool/client_fn_diff.py VisShop

Prints every method of the class with status:
  IDENTICAL   hash-identical twin in client - current C++ is already client-correct
  DIFFERS     paired (vtable slot / name) but bodies differ - needs client port
  SERVER_ONLY no client counterpart found (server-newer, or pairing gap)
  CLIENT_ONLY client method without server counterpart
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

UNNAMED = re.compile(r'^(FUN_|sub_|thunk_|entry|AddrTable|switchD|nullsub|j_|unknown_libname|loc_)')


def load(path):
    return json.load(open(path))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('cls')
    a = ap.parse_args()
    cls = a.cls

    cm = load(os.path.join(STATE, 'client_classmap.json'))
    srv = load(os.path.join(BUILD, 'client_diff_snapshot_a2serv6_exe.json'))
    cli = load(os.path.join(BUILD, 'client_diff_snapshot_allods2_exe.json'))
    srv_h = {int(it['entry'], 16): it for it in load(os.path.join(BUILD, 'func_hashes_server.json'))}
    cli_h = {int(it['entry'], 16): it for it in load(os.path.join(BUILD, 'func_hashes_client.json'))}
    srv_funcs = {f['va']: f for f in srv['functions']}
    cli_funcs = {f['va']: f for f in cli['functions']}

    rec = cm['classes'].get(cls)
    if rec is None:
        print('class not in map:', cls)
        return
    print('class %s  bases=%s  ctor %s -> %s' % (
        cls, ','.join(rec['bases']) or '-', rec['ctor_srv'] or '-', rec['ctor_cli'] or '-'))
    for va, b, n in rec['srv_vts']:
        print('  server vtable %s (%d slots, %s)' % (va, n, b))
    for va, b, n in rec['cli_vts']:
        print('  client vtable %s (%d slots, %s)' % (va, n, b))

    # rebuild vtable pieces
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

    # member sets
    srv_members = {va for va, f in srv_funcs.items() if f['ns'] and f['ns'][-1].rstrip('_conflict0123456789') == cls}
    for va, b, n in rec['srv_vts']:
        srv_members.update(srv_pieces.get(int(va, 16), []))
    cli_members = set()
    for va, b, n in rec['cli_vts']:
        cli_members.update(cli_pieces.get(int(va, 16), []))
    if rec['ctor_cli']:
        cli_members.add(int(rec['ctor_cli'], 16))
    for e in cm['callsite_pairs']:
        if e['server_class'] == cls:
            cli_members.add(int(e['client'], 16))
            srv_members.add(int(e['server'], 16))

    cli_shape = defaultdict(list)
    for va, h in cli_h.items():
        cli_shape[h['h_shape']].append(va)

    used_s, used_c = set(), set()
    rows = []

    # 1. hash-identical (class-scoped, server va used once)
    for cva in sorted(cli_members):
        h = cli_h.get(cva)
        if not h:
            continue
        twins = [s for s in srv_members if s not in used_s and s in srv_h
                 and srv_h[s]['h_shape'] == h['h_shape']]
        if twins:
            rows.append(('IDENTICAL', 'hash', twins[0], cva))
            used_s.add(twins[0])
            used_c.add(cva)

    # 2. vtable slot alignment
    for sva_rec, cva_rec in zip(rec['srv_vts'], rec['cli_vts']):
        ss = srv_pieces.get(int(sva_rec[0], 16), [])
        cs = cli_pieces.get(int(cva_rec[0], 16), [])
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
            rows.append(('DIFFERS' if srv_h.get(sva, {}).get('h_shape') != cli_h.get(cva, {}).get('h_shape') else 'IDENTICAL',
                         'slot%d' % i, sva, cva))
            used_s.add(sva)
            used_c.add(cva)

    # 3. call-site pairs from the classmap (identical callers, aligned call sites)
    for e in cm['callsite_pairs']:
        sva = int(e['server'], 16)
        cva = int(e['client'], 16)
        if sva in srv_members and cva in cli_members and sva not in used_s and cva not in used_c:
            rows.append(('DIFFERS', 'callsite:%d votes' % e['votes'], sva, cva))
            used_s.add(sva)
            used_c.add(cva)

    # 4. names (server members real-named, client members from REA/pushed names)
    cli_named = {}
    for v in cli_members - used_c:
        f = cli_funcs.get(v)
        if f and f['name'] and not UNNAMED.match(f['name']):
            cli_named.setdefault(f['name'], []).append(v)
    for sva in sorted(srv_members - used_s):
        f = srv_funcs.get(sva)
        if not f or UNNAMED.match(f['name']):
            continue
        cands = [v for v in cli_named.get(f['name'], []) if v not in used_c]
        if cands:
            s_sz = max(f['size'], 1)
            cva = min(cands, key=lambda v: abs((cli_funcs[v]['size'] or 1) - s_sz) / s_sz)
            rows.append(('DIFFERS', 'name', sva, cva))
            used_s.add(sva)
            used_c.add(cva)

    srv_shape_map = defaultdict(list)
    for s, h in srv_h.items():
        srv_shape_map[h['h_shape']].append(s)
    cli_shape_map = defaultdict(list)
    for c, h in cli_h.items():
        cli_shape_map[h['h_shape']].append(c)

    for sva in sorted(srv_members - used_s):
        h = srv_h.get(sva)
        twin = cli_shape_map.get(h['h_shape'], [None])[0] if h else None
        if twin is not None:
            rows.append(('IDENTICAL', 'global-hash (client twin outside class anchors)', sva, twin))
            used_c.discard(twin)
            continue
        rows.append(('SERVER_ONLY', 'ns/vt', sva, None))
    for cva in sorted(cli_members - used_c):
        rows.append(('CLIENT_ONLY', 'vt', None, cva))

    def nm(va, side):
        f = (srv_funcs if side == 's' else cli_funcs).get(va)
        return f['name'] if f else '?'

    order = {'IDENTICAL': 0, 'DIFFERS': 1, 'CLIENT_ONLY': 2, 'SERVER_ONLY': 3}
    rows.sort(key=lambda r: (order[r[0]], -(srv_funcs.get(r[2], {}).get('size', 0) or 0)))
    print()
    print('%-12s %-28s %-9s %-9s %-7s %-7s %s' % ('status', 'method (server name)', 'srv', 'cli', 'srv_sz', 'cli_sz', 'basis'))
    out = []
    for status, basis, sva, cva in rows:
        name = nm(sva, 's') if sva else nm(cva, 'c')
        print('%-12s %-28s %-9s %-9s %-7d %-7d %s' % (
            status, name[:28],
            '%06x' % sva if sva else '-', '%06x' % cva if cva else '-',
            srv_funcs.get(sva, {}).get('size', 0), cli_funcs.get(cva, {}).get('size', 0), basis))
        out.append({'status': status, 'method': name, 'basis': basis,
                    'server_va': '%06x' % sva if sva else None, 'client_va': '%06x' % cva if cva else None,
                    'server_size': srv_funcs.get(sva, {}).get('size', 0),
                    'client_size': cli_funcs.get(cva, {}).get('size', 0)})
    from collections import Counter
    print()
    print(dict(Counter(r[0] for r in rows)))
    with open(os.path.join(BUILD, 'fn_diff_%s.json' % cls), 'w') as fh:
        json.dump(out, fh, indent=1)


if __name__ == '__main__':
    main()
