#!/usr/bin/env python
"""Definitive server<->client class map (a2serv6.exe <-> allods2.exe).

Layers of evidence, strongest first:

  ctor-ns      server vtable labeled by the namespace(s) of code that references
               it (constructors write the vtable pointer) - definitive
  ctor-twin    client vtable labeled via a client ctor whose body hash-matches
               a server ctor of known class - definitive
  vtable-match structural vtable pairing (multiset of slot hashes), now on
               code-reference-split pieces instead of raw runs
  slot-twin    client vtable labeled by majority namespace of its slots' hash twins
  callsite     function pairs implied by aligned call sites inside hash-identical
               caller pairs (k-th direct call <-> k-th direct call), >=2 votes
  strings      shared rare string-literal fingerprints (weak, low confidence)

Also cross-checks the inheritance chain: a matched class's ctor should call the
base class ctor, on both sides, consistently with `class X : public Y` in the
repo headers.

Inputs:  build/client_diff_snapshot_*.json + build/func_hashes_*.json
Output:  migration_state/client_classmap.json + client_classmap.md

Usage:   python tool/client_classmap.py
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


def real(name):
    return bool(name) and not UNNAMED.match(name)


def norm_cls(name):
    """Ghidra namespace collision variants ('X_conflict1') are the same class."""
    return re.sub(r'_conflict\d+$', '', name) if name else name


def load_hashes(path):
    return {int(it['entry'], 16): it for it in json.load(open(path))}


class Side:
    def __init__(self, snap_path, hash_path):
        snap = json.load(open(snap_path))
        self.name = snap['program']
        self.funcs = {f['va']: f for f in snap['functions']}
        self.hashes = load_hashes(hash_path)
        self.calls = {int(k, 16): v for k, v in snap['calls'].items()}
        self.datarefs = {int(k, 16): v for k, v in snap['datarefs'].items()}
        self.strings = {int(k, 16): v for k, v in snap['strings'].items()}
        self.runs = snap['vtables']
        self.refs_to = defaultdict(list)
        for fn, tos in self.datarefs.items():
            for to in tos:
                self.refs_to[to].append(fn)
        # unique hash -> va on this side (unambiguous twins only)
        by_shape = defaultdict(list)
        for va, h in self.hashes.items():
            by_shape[h['h_shape']].append(va)
        self.unique = {s: v[0] for s, v in by_shape.items() if len(v) == 1}

    def twin_of(self, va, other):
        h = self.hashes.get(va)
        if not h:
            return None
        return other.unique.get(h['h_shape'])

    def resolve(self, va):
        """Follow thunk chains to the real function."""
        for _ in range(4):
            f = self.funcs.get(va)
            if f and f.get('thunk_to'):
                va = f['thunk_to']
            else:
                break
        return va

    def fn_strings(self, va):
        return {self.strings[t] for t in self.datarefs.get(va, []) if t in self.strings}

    def ns(self, va):
        f = self.funcs.get(va)
        return f['ns'][-1] if f and f['ns'] else None


def split_runs(runs, refs_to):
    """Split vtable runs at code-referenced interior offsets (adjacent vtables
    with no separating dword merge in a raw scan; ctors reference each start)."""
    pieces = []
    for run in runs:
        va, slots = run['va'], run['slots']
        cuts = {0}
        for off in range(4, len(slots) * 4, 4):
            if (va + off) in refs_to:
                cuts.add(off // 4)
        cuts = sorted(cuts)
        for i, c in enumerate(cuts):
            end = cuts[i + 1] if i + 1 < len(cuts) else len(slots)
            if end - c < 2:  # a <2-slot tail is not a vtable
                continue
            pieces.append({'va': va + c * 4, 'slots': slots[c:end], 'rtti': run['rtti'] if c == 0 else None,
                           'refs': sorted(set(refs_to.get(va + c * 4, [])))})
    return pieces


def structural_match(srv_pieces, cli_pieces, srv, cli, min_shared=3, min_ratio=0.34):
    cand = []
    for si, sp in enumerate(srv_pieces):
        sh = Counter(srv.hashes[s]['h_shape'] for s in sp['slots'] if s in srv.hashes)
        if not sh:
            continue
        for ci, cp in enumerate(cli_pieces):
            ch = Counter(cli.hashes[c]['h_shape'] for c in cp['slots'] if c in cli.hashes)
            if not ch:
                continue
            shared = sum((sh & ch).values())
            if shared >= min_shared and shared >= min_ratio * min(len(sp['slots']), len(cp['slots'])):
                cand.append((shared, si, ci))
    cand.sort(key=lambda x: -x[0])
    used_s, used_c, pairs = set(), set(), []
    for shared, si, ci in cand:
        if si in used_s or ci in used_c:
            continue
        used_s.add(si)
        used_c.add(ci)
        pairs.append((si, ci, shared))
    return pairs


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--votes', type=int, default=2, help='min call-site votes for a pair')
    a = ap.parse_args()

    srv = Side(SRV_SNAP, SRV_HASH)
    cli = Side(CLI_SNAP, CLI_HASH)
    print('loaded %s (%d fns) / %s (%d fns)' % (srv.name, len(srv.funcs), cli.name, len(cli.funcs)))

    # --- 1. split + label vtable pieces --------------------------------------
    srv_pieces = split_runs(srv.runs, srv.refs_to)
    cli_pieces = split_runs(cli.runs, cli.refs_to)
    print('vtable pieces: server %d (from %d runs), client %d (from %d runs)' % (
        len(srv_pieces), len(srv.runs), len(cli_pieces), len(cli.runs)))

    def label_srv_piece(p):
        ns_votes = Counter(srv.ns(r) for r in p['refs'] if srv.ns(r))
        if ns_votes:
            top, n = ns_votes.most_common(1)[0]
            p['label'] = top
            p['label_basis'] = 'ctor-ns%d' % n
            return
        ns_votes = Counter(srv.ns(s) for s in p['slots'] if srv.ns(s))
        if ns_votes:
            p['label'] = ns_votes.most_common(1)[0][0]
            p['label_basis'] = 'slot-ns'
            return
        if p['rtti'] and p['rtti'].startswith('.?A'):
            p['label'] = p['rtti'][4:].rstrip('@').split('@')[0]
            p['label_basis'] = 'rtti'

    def label_cli_piece(p):
        # client ctor candidate hash-matches a server ctor of a known class?
        for r in p['refs']:
            t = cli.twin_of(r, srv)
            if t is not None and srv.ns(t):
                p['label'] = srv.ns(t)
                p['label_basis'] = 'ctor-twin'
                return
        votes = Counter()
        for c in p['slots']:
            t = cli.twin_of(c, srv)
            if t is not None and srv.ns(t):
                votes[srv.ns(t)] += 1
        if votes:
            top, n = votes.most_common(1)[0]
            p['label'] = top
            p['label_basis'] = 'slot-twin%d' % n
            return
        ns_votes = Counter(cli.ns(c) for c in p['slots'] if cli.ns(c))
        if ns_votes:
            p['label'] = ns_votes.most_common(1)[0][0]
            p['label_basis'] = 'cli-ns'
            return
        if p['rtti'] and p['rtti'].startswith('.?A'):
            p['label'] = p['rtti'][4:].rstrip('@').split('@')[0]
            p['label_basis'] = 'rtti'

    for p in srv_pieces:
        label_srv_piece(p)
    for p in cli_pieces:
        label_cli_piece(p)

    # --- 2. match pieces: structural + ctor-anchored --------------------------
    vt_pairs = {}  # (si, ci) -> set(evidence)
    for si, ci, shared in structural_match(srv_pieces, cli_pieces, srv, cli):
        vt_pairs[(si, ci)] = {'struct:%d' % shared}
    # ctor anchor: server piece's referencing ctor hash-twins a client fn that
    # references this client piece
    used_ci = {ci for (_, ci) in vt_pairs}
    for si, sp in enumerate(srv_pieces):
        for r in sp['refs']:
            t = srv.twin_of(r, cli)
            if t is None:
                continue
            for to in cli.datarefs.get(t, []):
                for ci, cp in enumerate(cli_pieces):
                    if cp['va'] == to and ci not in used_ci:
                        vt_pairs[(si, ci)] = {'ctor'}
                        used_ci.add(ci)
    print('vtable piece pairs: %d' % len(vt_pairs))

    # --- 3. call-site propagation ---------------------------------------------
    votes = Counter()
    for shape, sva in srv.unique.items():
        cva = cli.unique.get(shape)
        if cva is None:
            continue  # identical twin only counts when unambiguous on both sides
        sc = srv.calls.get(sva)
        cc = cli.calls.get(cva)
        if not sc or not cc or len(sc) != len(cc):
            continue
        for ts, tcc in zip(sc, cc):
            if ts == sva or tcc == cva:
                continue
            if srv.hashes.get(ts, {}).get('h_shape') == cli.hashes.get(tcc, {}).get('h_shape'):
                continue  # already hash-paired
            votes[(ts, tcc)] += 1
    callsite_pairs = {p for p, v in votes.items() if v >= a.votes}
    print('call-site pairs (>= %d votes): %d' % (a.votes, len(callsite_pairs)))

    # --- 4. string fingerprints ------------------------------------------------
    df = Counter()
    for side in (srv, cli):
        for fn in side.datarefs:
            for s in side.fn_strings(fn):
                df[s] += 1
    srv_str_index = defaultdict(list)  # string -> server fn vas (rare strings only)
    for fn in srv.datarefs:
        for s in srv.fn_strings(fn):
            if df[s] <= 8:
                srv_str_index[s].append(fn)
    cli_candidates = {}
    for fn in cli.datarefs:
        strs = {s for s in cli.fn_strings(fn) if df[s] <= 8}
        if len(strs) < 2:
            continue
        score = Counter()
        for s in strs:
            for cand in srv_str_index.get(s, []):
                score[cand] += 1
        if not score:
            continue
        best, hits = score.most_common(1)[0]
        if hits >= 2:
            cli_candidates[fn] = (best, hits, len(strs))
    print('string-fingerprint candidate pairs: %d' % len(cli_candidates))

    # --- 5. class map assembly --------------------------------------------------
    bases = {}   # class -> [base classes] from headers
    repo = defaultdict(set)
    pat_cls = re.compile(r'^\s*(?:class|struct)\s+([A-Za-z_]\w*)\s*(?::\s*(.+))?\s*\{?\s*$|^\s*(?:class|struct)\s+([A-Za-z_]\w*)\s*$')
    pat_pub = re.compile(r'public\s+([A-Za-z_]\w*)')
    for path in glob.glob(os.path.join(ROOT, 'src', '*.h')):
        rel = os.path.basename(path)
        for ln in open(path, encoding='utf-8', errors='replace'):
            m = re.match(r'^\s*(?:class|struct)\s+([A-Za-z_]\w*)\s*(?::([^{;]*))?', ln)
            if m:
                repo[m.group(1)].add(rel)
                if m.group(2):
                    bases[m.group(1)] = pat_pub.findall(m.group(2))

    # server class universe: namespaces + piece labels
    classes = {}
    for va, f in srv.funcs.items():
        if f['ns']:
            classes.setdefault(norm_cls(f['ns'][-1]), {'srv_vts': [], 'cli_vts': [], 'ctor': None,
                                                       'counts': Counter(), 'evidence': set()})
    classes_vis_like = set()

    def cls_rec(name):
        name = norm_cls(name)
        return classes.setdefault(name, {'srv_vts': [], 'cli_vts': [], 'ctor': None,
                                         'counts': Counter(), 'evidence': set()})

    for si, sp in enumerate(srv_pieces):
        if sp.get('label'):
            cls_rec(sp['label'])['srv_vts'].append((sp['va'], sp['label_basis'], len(sp['slots']), si))
    for (si, ci), ev in vt_pairs.items():
        sp, cp = srv_pieces[si], cli_pieces[ci]
        lbl = sp.get('label') or cp.get('label')
        if not lbl:
            continue
        rec = cls_rec(lbl)
        rec['cli_vts'].append((cp['va'], cp.get('label_basis'), len(cp['slots']), ci))
        rec['evidence'] |= ev
        rec['evidence'].add('vtable')

    # ctor pairs: among vtable referrers (ctor + dtors all write the vtable ptr)
    # prefer the real constructor (ns==class, name==class), then any class method,
    # then any referrer with an identical twin
    for name, rec in classes.items():
        for (va, basis, nslots, si) in rec['srv_vts']:
            best = None
            for r in srv_pieces[si]['refs']:
                f = srv.funcs.get(r)
                if not f:
                    continue
                score = 0
                if f['ns'] and norm_cls(f['ns'][-1]) == name:
                    score = 2
                    if f['name'] == name:
                        score = 3  # real ctor: name == class name
                    elif f['name'].startswith('~'):
                        score = 1  # dtor
                if score and (best is None or score > best[0]):
                    best = (score, r)
            if best is None:
                for r in srv_pieces[si]['refs']:
                    if srv.twin_of(r, cli) is not None:
                        best = (0, r)
                        break
            if best:
                t = srv.twin_of(best[1], cli)
                if t is not None:
                    rec['ctor'] = (best[1], t)
                    rec['ctor_basis'] = 'ctor' if best[0] >= 3 else ('method' if best[0] >= 1 else 'twin')
                    break

    # method counts per class: identical/differs/only via global hashes
    cli_shapes = Counter(h['h_shape'] for h in cli.hashes.values())
    for va, f in srv.funcs.items():
        if not f['ns']:
            continue
        rec = cls_rec(f['ns'][-1])
        h = srv.hashes.get(va)
        if h and cli_shapes.get(h['h_shape']):
            rec['counts']['identical'] += 1
        else:
            rec['counts']['server_only'] += 1
    for (ts, tcc) in callsite_pairs:
        ns = srv.ns(ts)
        if ns:
            cls_rec(ns)['counts']['callsite_pair'] += 1

    # hierarchy cross-check
    cli_shape_multi = defaultdict(list)
    for va, h in cli.hashes.items():
        cli_shape_multi[h['h_shape']].append(va)
    hierarchy = {}
    for name, rec in classes.items():
        hb = bases.get(name)
        if not hb or not rec['ctor']:
            continue
        s_ctor, c_ctor = rec['ctor']
        s_calls = [srv.resolve(t) for t in srv.calls.get(s_ctor, [])]
        c_calls = [cli.resolve(t) for t in cli.calls.get(c_ctor, [])]
        for b in hb:
            base_ctors = [t for t in s_calls if srv.ns(t) == b]
            if not base_ctors:
                continue
            ok = False
            for bc in base_ctors:
                h = srv.hashes.get(bc)
                twins = cli_shape_multi.get(h['h_shape'], []) if h else []
                for x in c_calls:
                    if x in twins or votes.get((bc, x)):
                        ok = True
                        break
                if ok:
                    break
            hierarchy[name] = {'base': b, 'ok': ok}

    # client-only vtable classes
    matched_ci = {ci for (si, ci) in vt_pairs}
    cli_only = []
    for ci, cp in enumerate(cli_pieces):
        if ci in matched_ci or not cp.get('label'):
            continue
        cli_only.append((cp['label'], cp['va'], cp['label_basis'], len(cp['slots'])))

    # --- 6. outputs --------------------------------------------------------------
    out = {
        'vtable_pairs': [{'class': srv_pieces[si].get('label') or cli_pieces[ci].get('label'),
                          'server_vt': '%06x' % srv_pieces[si]['va'],
                          'client_vt': '%06x' % cli_pieces[ci]['va'],
                          'evidence': sorted(ev),
                          'server_basis': srv_pieces[si].get('label_basis'),
                          'client_basis': cli_pieces[ci].get('label_basis'),
                          'slots_srv': len(srv_pieces[si]['slots']),
                          'slots_cli': len(cli_pieces[ci]['slots'])}
                         for (si, ci), ev in vt_pairs.items()],
        'classes': {name: {'repo': sorted(v), 'bases': bases.get(name, []),
                           'srv_vts': [('%06x' % va, b, n) for va, b, n, _ in rec['srv_vts']],
                           'cli_vts': [('%06x' % va, b, n) for va, b, n, _ in rec['cli_vts']],
                           'ctor_srv': '%06x' % rec['ctor'][0] if rec['ctor'] else None,
                           'ctor_cli': '%06x' % rec['ctor'][1] if rec['ctor'] else None,
                           'evidence': sorted(rec['evidence']),
                           'counts': dict(rec['counts']),
                           'hierarchy': hierarchy.get(name)}
                    for name, rec in classes.items() for v in [repo.get(name, set())]},
        'callsite_pairs': [{'server': '%06x' % s, 'client': '%06x' % c, 'votes': votes[(s, c)],
                            'server_name': srv.funcs.get(s, {}).get('name'),
                            'server_class': srv.ns(s)}
                           for (s, c) in sorted(callsite_pairs)],
        'string_pairs': [{'client': '%06x' % c, 'server': '%06x' % s, 'hits': h, 'cli_strings': n}
                         for c, (s, h, n) in cli_candidates.items()],
        'client_only_vtables': [{'label': l, 'va': '%06x' % va, 'basis': b, 'slots': n}
                                for l, va, b, n in cli_only],
    }
    with open(os.path.join(STATE, 'client_classmap.json'), 'w') as fh:
        json.dump(out, fh, indent=1)

    # markdown
    MFC = {'CWnd', 'CCmdTarget', 'CObject', 'CDialog', 'CFrameWnd', 'CView', 'CDocument',
           'CWinApp', 'CWinThread', 'CFile', 'CStdioFile', 'CException', 'CMemoryException',
           'CNotSupportedException', 'CByteArray', 'CWordArray', 'CPtrArray', 'CObArray',
           'CStringArray', 'CDWordArray', 'CUIntArray', 'CMapWordToPtr', 'CMapPtrToPtr',
           'CMapStringToOb', 'CMapStringToString', 'CMapStringToPtr', 'CListBox', 'CButton',
           'CEdit', 'CStatic', 'CComboBox', 'CStatusBar', 'CControlBar', 'CDialogBar',
           'CMemFile', 'CMirrorFile', 'CFileFind', 'CRecentFileList', 'CCmdUI', 'CDockContext',
           'CArchive', 'CFileException', 'CResourceException', 'CUserException',
           'CCommandLineInfo', 'CStatusCmdUI', 'CMiniDockFrameWnd', 'CSharedPtr',
           'CArray', 'CList', 'CMap', 'CString', 'CPoint', 'CRect', 'CSize', 'filebuf'}
    rows = []
    for name, rec in classes.items():
        if not rec['srv_vts'] and not rec['cli_vts'] and not rec['counts']:
            continue
        ev = sorted(rec['evidence'])
        c = rec['counts']
        total = c.get('identical', 0) + c.get('server_only', 0)
        if 'vtable' in ev:
            conf = 'HIGH'
        elif total >= 5 and c.get('identical', 0) >= 0.8 * total:
            conf = 'HIGH'  # non-polymorphic class proven identical by member hashes
        elif c.get('callsite_pair', 0) >= 3 or (total >= 5 and c.get('identical', 0) >= 0.5 * total):
            conf = 'MED'
        elif total == 0 and c.get('callsite_pair', 0):
            conf = 'MED'
        else:
            conf = 'LOW'
        rows.append((name, rec, conf))
    rows.sort(key=lambda r: (not r[0].startswith('Vis'), r[0]))
    lines = ['# Client/server class map (auto-generated by tool/client_classmap.py)', '',
             'Confidence: HIGH = vtable/ctor evidence, or >=80% of members hash-identical;',
             'MED = call-site pairs or >=50% identical; LOW = namespace only (likely server-only).', '',
             '| class | headers | base | server vtable(s) | client vtable(s) | ctor srv->cli | id/srv-only/callpair | ev | conf |',
             '|---|---|---|---|---|---|---|---|---|']
    for name, rec, conf in rows:
        c = rec['counts']
        svt = ' '.join('%06x(%s)' % (int(va), b) for va, b, _, _ in rec['srv_vts']) or '-'
        cvt = ' '.join('%06x(%s)' % (int(va), b) for va, b, _, _ in rec['cli_vts']) or '-'
        ctor = '%06x->%06x' % rec['ctor'] if rec['ctor'] else '-'
        lines.append('| %s | %s | %s | %s | %s | %s | %d/%d/%d | %s | %s |' % (
            name, ','.join(sorted(repo.get(name, []))) or '-', ','.join(bases.get(name, [])) or '-',
            svt, cvt, ctor, c.get('identical', 0), c.get('server_only', 0),
            c.get('callsite_pair', 0), '+'.join(rec['evidence']) or '-', conf))
    lines.append('')
    game_cli_only = [e for e in cli_only if e[0] not in MFC]
    lib_cli_only = [e for e in cli_only if e[0] in MFC]
    lines.append('Client-only vtable classes (game): %d; MFC/lib noise: %d. Details in client_classmap.json.' % (
        len(game_cli_only), len(lib_cli_only)))
    for l, va, b, n in sorted(game_cli_only, key=lambda x: -x[3])[:20]:
        lines.append('  - %s  cli_vt=%s  %s  slots=%d' % (l, va, b, n))
    lines.append('Call-site pairs: %d; string-fingerprint candidates: %d.' % (len(callsite_pairs), len(cli_candidates)))
    with open(os.path.join(STATE, 'client_classmap.md'), 'w') as fh:
        fh.write('\n'.join(lines) + '\n')

    n_hi = sum(1 for _, _, c in rows if c == 'HIGH')
    print('classes total: %d (HIGH %d, MED %d, LOW %d)' % (len(rows), n_hi, sum(1 for _, _, c in rows if c == 'MED'), sum(1 for _, _, c in rows if c == 'LOW')))
    print('client-only vtable classes: %d game, %d MFC/lib' % (len(game_cli_only), len(lib_cli_only)))
    print('wrote migration_state/client_classmap.json + client_classmap.md')


if __name__ == '__main__':
    main()
