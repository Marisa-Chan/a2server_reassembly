#!/usr/bin/env python
"""Annotate header declarations with their vanilla-client twin address.

Rewrites `// <server_va>` comments into `// <server_va> (client: <client_va>)`
for every function whose server<->client pairing is vetted:
  - the pair set from migration_state/client_diff.json (worklist),
  - kept only when the mapping is 1:1 in BOTH directions (many shape-hash
    collisions claim one client stub for several server functions),
  - and the RET-immediate sets of both sides agree (build/ret_map_*.json;
    h_shape excludes immediates, so even "IDENTICAL" pairs can differ in
    stack-arg cleanup - those are signature differences or bad pairs).

Existing hand-written markers are normalized into the same inline format.
No Ghidra needed; run from the repo root:

    python tool/add_client_refs.py --dry-run    # preview
    python tool/add_client_refs.py              # apply
"""
import argparse
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
ROOT = os.path.dirname(HERE)

import ghidra_backport as bp  # parse_headers for decl addresses

WL = os.path.join(ROOT, 'migration_state', 'client_diff.json')
RET_SRV = os.path.join(ROOT, 'build', 'ret_map_srv.json')
RET_CLI = os.path.join(ROOT, 'build', 'ret_map_cli.json')

INLINE = '(client: %s)'


def ret_imm_set(rets):
    out = set()
    for r in rets:
        if r.strip() == 'RET':
            out.add(0)
            continue
        m = re.search(r'RET\s+(?:dword\s+)?0*x?([0-9A-Fa-f]+)h?', r)
        out.add(int(m.group(1), 16) if m else -1)
    return out


def vetted_pairs():
    wl = json.load(open(WL))['worklist']
    by_srv, by_cli = {}, {}
    for it in wl:
        if it.get('server_va') and it.get('client_va') and it['status'] in ('IDENTICAL', 'DIFFERS'):
            by_srv.setdefault(it['server_va'], set()).add(it['client_va'])
            by_cli.setdefault(it['client_va'], set()).add(it['server_va'])
    rs = json.load(open(RET_SRV))
    rc = json.load(open(RET_CLI))
    pairs, dropped = {}, {'not-1:1': 0, 'ret-mismatch': 0, 'no-ret-data': 0}
    for sva, clis in by_srv.items():
        if len(clis) != 1:
            dropped['not-1:1'] += 1
            continue
        cva = next(iter(clis))
        if len(by_cli[cva]) != 1:
            dropped['not-1:1'] += 1
            continue
        s, c = rs.get(sva), rc.get(cva)
        if not s or not c:
            dropped['no-ret-data'] += 1
        elif ret_imm_set(s) != ret_imm_set(c) or -1 in ret_imm_set(s) | ret_imm_set(c):
            dropped['ret-mismatch'] += 1
            continue
        pairs[sva] = cva
    return pairs, dropped


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--dry-run', action='store_true')
    a = ap.parse_args()

    pairs, dropped = vetted_pairs()
    print('vetted pairs: %d  dropped: %s' % (len(pairs), dropped))

    headers = sorted(os.path.join(ROOT, 'src', f) for f in os.listdir(os.path.join(ROOT, 'src')) if f.endswith('.h'))
    decls = [d for d in bp.parse_headers(headers) if d.get('addr')]
    print('declarations with addresses: %d' % len(decls))

    files = {}
    stats = {'marked': 0, 'already': 0, 'no-pair': 0, 'name-addr': 0, 'normalized': 0}
    for d in decls:
        sva = d['addr'].lower()
        cva = pairs.get(sva)
        if cva is None:
            stats['no-pair'] += 1
            continue
        path = os.path.join(ROOT, d['file'])
        if path not in files:
            files[path] = open(path, encoding='utf-8', errors='surrogateescape').read().splitlines()
        lines = files[path]
        i = d['line'] - 1
        raw = lines[i]
        if '(client:' in raw:
            stats['already'] += 1
            continue
        comment_at = raw.find('//')
        if comment_at < 0:
            # address lives in the function name (sub_XXXXXX / FUN_00XXXXXX)
            lines[i] = raw.rstrip() + '  // client: ' + cva
            stats['name-addr'] += 1
            stats['marked'] += 1
            continue
        code, comment = raw[:comment_at], raw[comment_at:]
        if re.search(r'(?<![\w])%s(?![\w])' % sva, comment):
            new = re.sub(r'(?<![\w])(%s)(?![\w])' % sva, r'\1 ' + (INLINE % cva), comment, count=1)
        else:
            new = comment.rstrip() + ' ' + (INLINE % cva)
        lines[i] = code + new
        # normalize an old separate-line `// client: <va>` marker right below
        if i + 1 < len(lines):
            m = re.match(r'^(\s*)//\s*client:\s*([0-9a-fA-F]{6})\b(.*)$', lines[i + 1])
            if m and m.group(2).lower() == cva:
                note = m.group(3).strip()
                if note:
                    lines[i] = lines[i].rstrip() + ' ' + note
                lines[i + 1] = None
                stats['normalized'] += 1
        stats['marked'] += 1

    for path, lines in files.items():
        lines = [l for l in lines if l is not None]
        if a.dry_run:
            continue
        raw = open(path, 'rb').read()
        crlf = raw.count(b'\r\n') > (raw.count(b'\n') - raw.count(b'\r\n'))
        with open(path, 'w', encoding='utf-8', errors='surrogateescape', newline='\r\n' if crlf else '\n') as fh:
            fh.write('\n'.join(lines) + '\n')
    print('stats: %s' % stats)
    if a.dry_run:
        for path, lines in list(files.items())[:3]:
            print('--- sample', os.path.basename(path))
            for l in lines:
                if l and '(client:' in l:
                    print('   ', l.strip()[:110])
                    break


if __name__ == '__main__':
    main()
