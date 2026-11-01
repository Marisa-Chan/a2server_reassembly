#!/usr/bin/env python
"""Client-reference annotation loop: give every migrated function its vanilla-client
(allods2.exe) twin address as a `(client: <va>)` marker the Ghidra bridge understands,
with a per-function recorded reason.

Every annotation must rest on concrete evidence checked for THAT function - never a
blind hash-pairing. Evidence hierarchy (strongest first):
  string      both sides reference the same unique string literal
  callgraph   callers/callees already verified as pairs call/are called by the candidate
  vtable      same vtable pair at the same slot index (client_classmap vtable_pairs)
  decompile   statement-level comparison of both decompilations (required for DIFFERS)
  verified-log a pair already recorded by the client-verify loop (client_verified.txt)

State:
  migration_state/client_reference.txt   one line per resolved function (append-only):
      <srv_va> <Class::name> client=<cli_va> <basis> : <reason>
      <srv_va> <Class::name> client=NONE <basis> : <reason why no client twin exists>

Usage (repo root):
  python tool/client_refs.py status
  python tool/client_refs.py backlog [--class Foo] [--status DIFFERS] [--limit 40]
  python tool/client_refs.py check <va>            # bridge view: would backport/sync apply it?
  python tool/client_refs.py resolve <va>          # full evidence bundle for one function
  python tool/client_refs.py strings <va> [--client <cva>]
  python tool/client_refs.py annotate <va> <cva> --reason "..."
  python tool/client_refs.py none <va> --reason "..."
"""
import argparse
import glob
import json
import os
import re
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
ROOT = os.path.dirname(HERE)

import ghidra_backport as bp  # noqa: E402  (parse_headers/parse_cpp_addrs = the bridge parser)

REF_LOG = os.path.join(ROOT, 'migration_state', 'client_reference.txt')
WL = os.path.join(ROOT, 'migration_state', 'client_diff.json')
CLASSMAP = os.path.join(ROOT, 'migration_state', 'client_classmap.json')
SRV_SNAP = os.path.join(ROOT, 'build', 'client_diff_snapshot_a2serv6_exe.json')
CLI_SNAP = os.path.join(ROOT, 'build', 'client_diff_snapshot_allods2_exe.json')
SRV_HASH = os.path.join(ROOT, 'build', 'func_hashes_server.json')
CLI_HASH = os.path.join(ROOT, 'build', 'func_hashes_client.json')
VER_LOG = os.path.join(ROOT, 'migration_state', 'client_verified.txt')
RET_SRV = os.path.join(ROOT, 'build', 'ret_map_srv.json')
RET_CLI = os.path.join(ROOT, 'build', 'ret_map_cli.json')


def ret_set(rets):
    out = set()
    for r in rets or []:
        if r.strip() == 'RET':
            out.add(0)
            continue
        m = re.search(r'RET\s+(?:dword\s+)?0*x?([0-9A-Fa-f]+)h?', r)
        out.add(int(m.group(1), 16) if m else -1)
    return out

INLINE = '(client: %s)'


def load_all():
    headers = sorted(glob.glob(os.path.join(ROOT, 'src', '*.h')))
    cpps = sorted(glob.glob(os.path.join(ROOT, 'src', '*.cpp')))
    decls = list(bp.parse_headers(headers))
    cpp_addrs = bp.parse_cpp_addrs(cpps)
    for d in decls:
        d['prio'] = 0
        if not d['addr']:
            d['addr'], d['prio'] = bp.lookup_cpp_addr(cpp_addrs, d)
    return decls


def norm(va):
    if va is None:
        return None
    if isinstance(va, int):
        return '%06x' % va
    return va.strip().lower().removeprefix('0x')


def _refs_to(snap):
    r = defaultdict(list)
    for fn, tos in snap['datarefs'].items():
        for to in tos:
            r[to].append(int(fn, 16))
    return r


def _split_runs(runs, refs_to):
    """vtable runs cut into pieces at referenced slots (same algorithm as client_label_push)"""
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
            pieces[norm(va + c * 4)] = slots[c:end]
    return pieces


class Ctx:
    def __init__(self):
        self.decls = load_all()
        self.by_va = {}
        for d in self.decls:
            if d['addr'] and norm(d['addr']) not in self.by_va:  # header addr wins over cpp-comment
                self.by_va[norm(d['addr'])] = d
        wl = json.load(open(WL))
        self.wl_rows = defaultdict(list)      # srv va -> items
        self.wl_by_cli = defaultdict(list)
        for it in wl['worklist']:
            if it.get('server_va'):
                self.wl_rows[norm(it['server_va'])].append(it)
            if it.get('client_va'):
                self.wl_by_cli[norm(it['client_va'])].append(it)
        self.srv_snap = json.load(open(SRV_SNAP))
        self.cli_snap = json.load(open(CLI_SNAP))
        self.strings_of = {}                  # (snap, va) -> [contents]
        self.fns_by_string = {}               # (snap, content) -> [vas]
        for tag, snap in (('srv', self.srv_snap), ('cli', self.cli_snap)):
            d = defaultdict(list)
            for fn, tos in snap['datarefs'].items():
                for to in tos:
                    s = snap['strings'].get('%x' % to)
                    if s is not None:
                        d[norm(fn)].append(s)
            m = defaultdict(list)
            for va, ss in d.items():
                for s in ss:
                    m[s].append(va)
            self.strings_of[tag] = d
            self.fns_by_string[tag] = m
        self.callers = {}                     # (tag, va) -> [caller vas]
        for tag, snap in (('srv', self.srv_snap), ('cli', self.cli_snap)):
            c = defaultdict(set)
            for fn, tos in snap['calls'].items():
                for to in tos:
                    c[norm(to)].add(norm(fn))
            self.callers[tag] = {k: sorted(v) for k, v in c.items()}
        self.callees = {tag: {norm(fn): sorted(norm(t) for t in tos)
                              for fn, tos in snap['calls'].items()}
                        for tag, snap in (('srv', self.srv_snap), ('cli', self.cli_snap))}
        # established pairs: header markers first, then the client-verify log
        self.verified = {}
        for d in self.decls:
            if d.get('addr') and d.get('client_addr'):
                self.verified[d['addr']] = d['client_addr']
        self.verified_src = {va: 'marker' for va in self.verified}
        if os.path.exists(VER_LOG):
            for line in open(VER_LOG, encoding='utf-8', errors='replace'):
                m = re.match(r'(IDENTICAL|PORTED)\s+\S+\s+\S+\s+([0-9A-Fa-f]{6})\s+([0-9A-Fa-f]{6})', line)
                if m:
                    s, c = norm(m.group(2)), norm(m.group(3))
                    if s not in self.verified:
                        self.verified[s] = c
                        self.verified_src[s] = 'client_verified:' + m.group(1)
        self.cli_verified = {c: s for s, c in self.verified.items()}
        # vtable pieces per program + slot lookup: va -> [(vt_piece_va, slot_index, slots)]
        self.vt_pieces = {}
        self.vt_slots = {}
        for tag, snap in (('srv', self.srv_snap), ('cli', self.cli_snap)):
            pieces = _split_runs(snap['vtables'], _refs_to(snap))
            slots = defaultdict(list)
            for pva, sl in pieces.items():
                for i, fn in enumerate(sl):
                    slots[norm(fn)].append((pva, i, sl))
            self.vt_pieces[tag] = pieces
            self.vt_slots[tag] = dict(slots)
        self.vt_pairs = []
        cm = json.load(open(CLASSMAP)) if os.path.exists(CLASSMAP) else {'vtable_pairs': []}
        for vp in cm.get('vtable_pairs', []):
            sv, cv = norm(vp['server_vt']), norm(vp['client_vt'])
            if sv in self.vt_pieces['srv'] and cv in self.vt_pieces['cli']:
                self.vt_pairs.append((vp.get('class') or '?', sv, cv))
        # body shape hashes (for slot-level confirmation)
        self.hashes = {}
        for tag, path in (('srv', SRV_HASH), ('cli', CLI_HASH)):
            try:
                self.hashes[tag] = {norm(int(it['entry'], 16)): it.get('h_shape') for it in json.load(open(path))}
            except OSError:
                self.hashes[tag] = {}
        self.records = {}
        if os.path.exists(REF_LOG):
            for line in open(REF_LOG, encoding='utf-8', errors='replace'):
                m = re.match(r'([0-9A-Fa-f]{6})\s+\S+\s+client=(NONE|[0-9A-Fa-f]{6})', line)
                if m:
                    self.records[norm(m.group(1))] = line.rstrip('\n')

    def backlog(self):
        """cli-visible decls with a server address but no marker and no record line"""
        out = []
        for d in self.decls:
            if not d['cli_vis'] or d.get('client_addr') or not d['addr']:
                continue
            if norm(d['addr']) in self.records:
                continue
            out.append(d)
        return out

    def scope_name(self, d):
        return (d['cls'] + '::' if d['cls'] else '') + d['name']


def safe(s):
    return str(s).encode('ascii', 'backslashreplace').decode('ascii')


def fmt_strings(ctx, tag, va, limit=8):
    ss = ctx.strings_of[tag].get(norm(va), [])
    uniq = sorted(set(ss))
    return [safe(s) for s in uniq[:limit]] + (['...'] if len(uniq) > limit else [])


def pair_string_anchors(ctx, sva, cva):
    """unique-in-both string literals referenced by both sides"""
    ss = set(ctx.strings_of['srv'].get(norm(sva), []))
    cs = set(ctx.strings_of['cli'].get(norm(cva), []))
    shared = []
    for s in ss & cs:
        if len(ctx.fns_by_string['srv'][s]) == 1 and len(ctx.fns_by_string['cli'][s]) == 1:
            shared.append(s)
    return sorted(shared)


def callgraph_agreement(ctx, sva, cva):
    """(callers-agree, callers-verified, callees-agree, callees-verified, mismatches)"""
    out = []
    for kind, table, other in (('caller', ctx.callers['srv'], None), ('callee', ctx.callees['srv'], None)):
        rel = table.get(norm(sva), [])
        ver = [x for x in rel if x in ctx.verified]
        agree = 0
        mism = []
        for x in ver:
            twin = ctx.verified[x]
            if kind == 'caller':
                ok = norm(cva) in ctx.callers['cli'].get(twin, [])
            else:
                ok = twin in ctx.callees['cli'].get(norm(cva), [])
            if ok:
                agree += 1
            else:
                src = '' if ctx.verified_src.get(x) == 'marker' else ' via-log'
                mism.append('%s %s->%s%s' % (kind, x, twin, src))
        out.append((agree, len(ver), mism))
    return out


def status(ctx):
    decls = ctx.decls
    with_addr = [d for d in decls if d['addr'] and d['cli_vis']]
    marked = [d for d in decls if d.get('client_addr')]
    bl = ctx.backlog()
    none_recs = sum(1 for r in ctx.records.values() if 'client=NONE' in r)
    done_recs = len(ctx.records) - none_recs
    print('declarations parsed:        %d' % len(decls))
    print('  with client marker:       %d' % len(marked))
    print('  cli-visible, w/ srv addr: %d' % len(with_addr))
    print('client_reference.txt:       %d lines (%d client=..., %d NONE)' % (len(ctx.records), done_recs, none_recs))
    print('backlog (no marker/rec):    %d' % len(bl))
    by_status = Counter()
    for d in bl:
        rows = ctx.wl_rows.get(norm(d['addr']), [])
        by_status[rows[0]['status'] if rows else 'NO-ROW'] += 1
    for k, v in by_status.most_common():
        print('   %-12s %d' % (k, v))


def backlog(ctx, a):
    bl = ctx.backlog()
    if a.cls:
        bl = [d for d in bl if d['cls'] == a.cls]
    bl.sort(key=lambda d: ((d['cls'] or '~'), d['name'], d['addr']))
    n = 0
    for d in bl:
        rows = ctx.wl_rows.get(norm(d['addr']), [])
        if a.status and not any(r['status'] == a.status for r in rows):
            continue
        cands = sorted({(r['client_va'] or '-', r['status'], r['basis']) for r in rows})
        cand_s = ' '.join('%s/%s/%s' % c for c in cands[:3]) + (' +%d' % (len(cands) - 3) if len(cands) > 3 else '')
        print('%s %-44s %s' % (d['addr'], ctx.scope_name(d), cand_s or 'NO-ROW'))
        n += 1
        if a.limit and n >= a.limit:
            break
    print('(%d shown)' % n)


def check(ctx, va):
    """the bridge's view: is this declaration part of backport/sync or not"""
    va = norm(va)
    d = ctx.by_va.get(va)
    if d is None:
        print('no declaration carries server address %s' % va.upper())
        return 1
    print('%s  %s [%s:%d]' % (va.upper(), ctx.scope_name(d), d['file'], d['line']))
    print('  visibility: server=%s client=%s' % (d['srv_vis'], d['cli_vis']))
    print('  backport SERVER: %s (addr %s, prio %d)' % ('APPLIED' if d['srv_vis'] else 'SKIPPED (A2CLIENT-only)', va.upper(), d['prio']))
    if d.get('client_addr'):
        print('  backport CLIENT: APPLIED at %s' % d['client_addr'])
    else:
        print('  backport CLIENT: SKIPPED (no client marker)')
    print('  struct sync:    class %s (headers afxmsg_/dplay/dplobby/music/mfc_templ_impl excluded; fieldless non-polymorphic skipped)' % (d['cls'] or '(none - free function)'))
    rec = ctx.records.get(va)
    print('  reference log:  %s' % (rec or 'no line yet'))
    return 0


def resolve(ctx, va):
    va = norm(va)
    d = ctx.by_va.get(va)
    if d is None:
        print('no declaration carries server address %s' % va.upper())
        return 1
    print('== %s %s [%s:%d]' % (va.upper(), ctx.scope_name(d), d['file'], d['line']))
    rows = ctx.wl_rows.get(va, [])
    if rows:
        print('worklist rows:')
        for r in rows:
            print('  %-10s %-10s cli=%s sizes %s/%s thunk=%s' % (
                r['status'], r['basis'], r.get('client_va'), r.get('server_size'), r.get('client_size'), r.get('server_thunk')))
    else:
        print('worklist rows: NONE (automation found no candidate)')
    if d.get('client_addr'):
        print('ALREADY MARKED client=%s' % d['client_addr'])
    vt = vtable_evidence(ctx, va)
    if vt:
        print('vtable slots:')
        for cls, sv, i, cv, cfn, same in vt:
            print('   %s slot %d (vt %s<->%s): client fn %s%s' % (
                cls, i, sv, cv, cfn or '(past piece end)', ' [hash-equal]' if same else ''))
    else:
        print('vtable slots: fn not in any paired vtable')
    print('server strings: %s' % fmt_strings(ctx, 'srv', va))
    cands = []
    for r in rows:
        cand = r.get('client_va')
        if cand:
            cand = norm(cand)
            if cand not in cands:
                cands.append(cand)
    try:
        rs = ret_set(json.load(open(RET_SRV)).get(va))
        rc = {cva2: ret_set(json.load(open(RET_CLI)).get(cva2)) for cva2 in cands}
    except OSError:
        rs, rc = None, {}
    for cva in cands:
        anchors = pair_string_anchors(ctx, va, cva)
        (ca, ct, cm), (ea, et, em) = callgraph_agreement(ctx, va, cva)
        claim = len(ctx.wl_by_cli.get(cva, []))
        vt_ok = any(row[4] == cva for row in vt)
        print('candidate %s: strings-shared-unique=%d callgraph=%d/%d callers %d/%d callees claimed-by=%d vt-slot-ok=%s ret=%s%s' % (
            cva, len(anchors), ca, ct, ea, et, claim, vt_ok,
            ('same' if rc.get(cva) == rs else 'DIFFERs') if rs is not None else '?',
            '' if rs is None or rc.get(cva) == rs else ' (srv %s vs cli %s)' % (sorted(rs), sorted(rc.get(cva, set())))))
        for s in anchors[:4]:
            print('   anchor-string %r' % safe(s))
        for m in cm + em:
            print('   mismatch %s' % m)
        print('   client strings: %s' % fmt_strings(ctx, 'cli', cva))
    rec = ctx.records.get(va)
    if rec:
        print('recorded: %s' % rec)
    return 0


def strings_cmd(ctx, a):
    va = norm(a.va)
    print('server %s: %s' % (va, fmt_strings(ctx, 'srv', va, 20)))
    if a.client:
        cva = norm(a.client)
        print('client %s: %s' % (cva, fmt_strings(ctx, 'cli', cva, 20)))
        shared = pair_string_anchors(ctx, va, cva)
        print('unique-shared: %s' % [safe(s) for s in shared])


def vtable_evidence(ctx, sva):
    """every paired vtable in which the server fn occupies a slot:
    [(class, server_vt, slot, client_vt, client_fn_at_slot, same_hash)]"""
    out = []
    for pva, i in sorted((p, i) for p, i, _ in ctx.vt_slots['srv'].get(norm(sva), [])):
        for cls, sv, cv in ctx.vt_pairs:
            if pva != sv:
                continue
            cslots = ctx.vt_pieces['cli'][cv]
            cfn = norm(cslots[i]) if i < len(cslots) else None
            same = None
            if cfn:
                h1 = ctx.hashes['srv'].get(norm(sva))
                h2 = ctx.hashes['cli'].get(cfn)
                same = bool(h1) and h1 == h2
            out.append((cls, pva, i, cv, cfn, same))
    return out


def write_marker(d, cva, dry=False):
    path = os.path.join(ROOT, d['file'])
    lines = open(path, encoding='utf-8', errors='surrogateescape').read().splitlines()
    i = d['line'] - 1
    raw = lines[i]
    m = re.search(r'\bclient:\s*([0-9A-Fa-f]{6})', raw)
    if m:
        if m.group(1).lower() == cva.lower():
            return True  # idempotent: this line already carries the same marker
        print('line already has a DIFFERENT client marker (%s): %s' % (m.group(1), raw.strip()))
        return False
    cpos = raw.find('//')
    if cpos < 0:
        lines[i] = raw.rstrip() + '  // client: ' + cva
    else:
        code, comment = raw[:cpos], raw[cpos:]
        placed = None
        for tok in filter(None, {d['addr'], d['addr'].lower(), d['addr'].upper()}):
            if re.search(r'(?<![\w])%s(?![\w])' % tok, comment):
                placed = re.sub(r'(?<![\w])(%s)(?![\w])' % tok, r'\1 ' + (INLINE % cva), comment, count=1)
                break
        lines[i] = code + (placed if placed is not None else comment.rstrip() + ' ' + (INLINE % cva))
    if dry:
        print('dry-run: %s' % lines[i].strip())
        return True
    raw_bytes = open(path, 'rb').read()
    crlf = raw_bytes.count(b'\r\n') > (raw_bytes.count(b'\n') - raw_bytes.count(b'\r\n'))
    with open(path, 'w', encoding='utf-8', errors='surrogateescape', newline='\r\n' if crlf else '\n') as fh:
        fh.write('\n'.join(lines) + '\n')
    print('marked %s:%d: %s' % (d['file'], d['line'], lines[i].strip()))
    return True


def record(ctx, d, cva, basis, reason, force=False):
    va = norm(d['addr'])
    old = ctx.records.get(va)
    if old is not None:
        old_none = 'client=NONE' in old
        if not force and not (old_none and cva.upper() != 'NONE'):
            print('already recorded: %s' % old[:100])
            return
        print('superseding: %s' % old[:100])
    line = '%s %-44s client=%s %s : %s' % (va, ctx.scope_name(d), cva, basis, reason)
    with open(REF_LOG, 'a', encoding='utf-8') as fh:
        fh.write(line + '\n')
    ctx.records[va] = line
    print('recorded: %s' % line)


def annotate(ctx, a):
    va = norm(a.va)
    d = ctx.by_va.get(va)
    if d is None:
        sys.exit('no declaration carries server address %s' % va.upper())
    if not d['cli_vis']:
        sys.exit('declaration is inside #ifndef A2CLIENT - it has no client build; use `none`')
    cva = norm(a.client_va)
    # mark every decl line carrying this va (overloads can share one binary fn)
    targets = [x for x in ctx.decls if norm(x['addr']) == va and x['cli_vis']]
    ok = True
    for d in targets:
        if not write_marker(d, cva, dry=a.dry_run):
            ok = False
    # self-check: the bridge must now see the marker on exactly this decl
    if not a.dry_run:
        fresh = [x for x in load_all() if norm(x['addr']) == va]
        hit = [x for x in fresh if x.get('client_addr') == cva.upper()]
        if len(hit) < 1 or len({(x['file'], x['line']) for x in hit}) != len(targets):
            sys.exit('SELF-CHECK FAILED: re-parse does not see client=%s on %s (got %s)' % (
                cva.upper(), va.upper(), [x.get('client_addr') for x in fresh]))
        d = hit[0]
    if not a.dry_run and ok:
        record(ctx, d, cva, a.basis, a.reason, force=getattr(a, 'force', False))
    return 0


def none_cmd(ctx, a):
    va = norm(a.va)
    d = ctx.by_va.get(va)
    if d is None:
        sys.exit('no declaration carries server address %s' % va.upper())
    if d.get('client_addr') and not getattr(a, 'force', False):
        sys.exit('declaration already has client=%s - remove it first if genuinely server-only' % d['client_addr'])
    record(ctx, d, 'NONE', a.basis, a.reason, force=getattr(a, 'force', False))
    return 0


def vtable_cmd(ctx, a):
    """per-class slot review table for backlog items: every vtable read + hash/size/name"""
    cli_names = {norm(x['va']): x for x in ctx.cli_snap['functions']}
    srv_names = {norm(x['va']): x for x in ctx.srv_snap['functions']}
    bl = [d for d in ctx.backlog() if d['cls'] == a.cls]
    if not bl:
        print('no backlog items for class %s' % a.cls)
        return 0
    for d in sorted(bl, key=lambda d: (d['addr'], d['name'])):
        va = norm(d['addr'])
        vt = vtable_evidence(ctx, va)
        ssz = srv_names.get(va, {}).get('size', '?')
        if not vt:
            print('%s %-30s %sb  NO-VTABLE-SLOT' % (va, d['name'], ssz))
            continue
        reads = []
        for cls, sv, i, cv, cfn, same in vt:
            cn = cli_names.get(cfn, {}) if cfn else {}
            reads.append('%s@%d=%s(%s,%sb)%s' % (cls[:14], i, cfn or 'PAST-END',
                                                 cn.get('name', '?')[:18], cn.get('size', '?'), '*' if same else ''))
        fns = {r.split('=')[1].split('(')[0] for r in reads if '=PAST-END' not in r}
        verdict = 'UNANIMOUS' if len(fns) == 1 else 'DISAGREE'
        print('%s %-30s %sb  %s  %s' % (va, d['name'], ssz, verdict, ' ; '.join(reads[:4]) + (' +%d' % (len(reads) - 4) if len(reads) > 4 else '')))
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest='cmd', required=True)
    sub.add_parser('status')
    b = sub.add_parser('backlog')
    b.add_argument('--class', dest='cls')
    b.add_argument('--status')
    b.add_argument('--limit', type=int, default=0)
    c = sub.add_parser('check')
    c.add_argument('va')
    r = sub.add_parser('resolve')
    r.add_argument('va')
    s = sub.add_parser('strings')
    s.add_argument('va')
    s.add_argument('--client')
    s.add_argument('--force', action='store_true', help='supersede an existing pair record (last line wins on load)')
    v = sub.add_parser('vtable')
    v.add_argument('cls')
    an = sub.add_parser('annotate')
    an.add_argument('va')
    an.add_argument('client_va')
    an.add_argument('--reason', required=True)
    an.add_argument('--basis', default='manual')
    an.add_argument('--force', action='store_true', help='supersede an existing record (last line wins on load)')
    an.add_argument('--dry-run', action='store_true')
    nn = sub.add_parser('none')
    nn.add_argument('va')
    nn.add_argument('--reason', required=True)
    nn.add_argument('--basis', default='server-only')
    nn.add_argument('--force', action='store_true', help='supersede an existing pair record')
    a = ap.parse_args()

    ctx = Ctx()
    if a.cmd == 'status':
        status(ctx)
    elif a.cmd == 'backlog':
        backlog(ctx, a)
    elif a.cmd == 'check':
        sys.exit(check(ctx, a.va) or 0)
    elif a.cmd == 'resolve':
        sys.exit(resolve(ctx, a.va) or 0)
    elif a.cmd == 'strings':
        strings_cmd(ctx, a)
    elif a.cmd == 'vtable':
        sys.exit(vtable_cmd(ctx, a) or 0)
    elif a.cmd == 'annotate':
        sys.exit(annotate(ctx, a) or 0)
    elif a.cmd == 'none':
        sys.exit(none_cmd(ctx, a) or 0)


if __name__ == '__main__':
    main()
