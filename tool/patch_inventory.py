#!/usr/bin/env python3
"""Patch inventory for the my-srvmgr / my-a2mgr binary patch pipelines.

The patch managers build patched exes from an original exe + three patch kinds:
  1. byte patches (.xck, "cOMPARe-to-cRACk" format: per-file-offset before/after bytes)
  2. hooks written by DLLInject (server.dis) / add_dll (client allods2.mp):
     a JMP/CALL at a VA into a DLL export (+ optional extra NOP bytes)
  3. a few runtime memcpy patches inside the DLLs (NOT visible in the exe files)

Ground truth for (1)+(2) composed together is the *release* exe the pipeline
produces: this tool byte-diffs original vs release per PE section, then attaches
hook/xck metadata, resolves every site to its containing function (function
extents from the Ghidra snapshots), and classifies:

  takeover   - active hook whose VA is a function entry: the original function
               body is dead and typically reimplemented inside the DLL
  cave site  - any hook/region inside a takeover's dead body (new code spliced
               into retail fragments the DLL re-enters, or scratch code)
  hook       - active JMP/CALL splice inside a live function
  region     - byte diff not covered by any hook (xck-only / injector data)

Outputs migration_state/patch_inventory.{json,md} and prints a summary.

Usage:  python tool/patch_inventory.py [--no-decode]
"""

import argparse
import fnmatch
import json
import os
import re
import struct
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRVMGR = os.environ.get('A2_SRVMGR', r'C:\Users\ikacn\Desktop\stuff\a2\my-srvmgr')
A2MGR = os.environ.get('A2_A2MGR', r'C:\Users\ikacn\Desktop\stuff\a2\my-a2mgr')

# Runtime-only memcpy patches (not visible in the exe diff); found by grep in the DLL sources.
RUNTIME_PATCHES = {
    'client': [
        {'va': 0x5D92F4, 'what': 'PSCRL_rep ptr write (later overwritten with PSCRL_shopShelfOnKeyDown)'},
        {'va': 0x5D8BAC, 'what': 'PSCRL_gameInventory ptr write'},
    ],
}

# Entry hooks whose DLL export FULLY reimplements the function (verified by reading the
# DLL source: the body is a modified copy of the original disassembly, and the original
# body is dead except for fragments the DLL jumps back into). Only these get "cave"
# classification for further sites inside the body; other entry hooks (imp_LoadingMap
# etc.) splice back into the live original flow.
KNOWN_TAKEOVERS = {
    'server': {
        'cheat_codes': 'cheat_codes.cpp is a full modified copy of Server::CheatCommand; '
                       'jumps back into intact fragments (locu_*), other hooks splice into them',
    },
    'client': {},
}

DIFF_GAP_MERGE = 8  # merge diff runs separated by <= this many equal bytes


def u16(data, off):
    return struct.unpack_from('<H', data, off)[0]


def u32(data, off):
    return struct.unpack_from('<I', data, off)[0]


def parse_pe(data):
    e_lfanew = u32(data, 0x3C)
    if data[e_lfanew:e_lfanew + 4] != b'PE\0\0':
        raise ValueError('not a PE file')
    coff = e_lfanew + 4
    machine, nsec, _ts, _ptr, _sym, opt_size, _chars = struct.unpack_from('<HHIIIHH', data, coff)
    opt = coff + 20
    magic = u16(data, opt)
    if magic != 0x10B:
        raise ValueError('not PE32')
    image_base = u32(data, opt + 28)
    numdirs = u32(data, opt + 92)
    dirs = [(u32(data, opt + 96 + 8 * i), u32(data, opt + 96 + 8 * i + 4)) for i in range(numdirs)]
    sec0 = opt + opt_size
    sections = []
    for i in range(nsec):
        off = sec0 + 40 * i
        name = data[off:off + 8].rstrip(b'\0').decode('latin1')
        vsize, vaddr, rsize, raddr = struct.unpack_from('<IIII', data, off + 8)
        sections.append({'name': name, 'vaddr': vaddr, 'vsize': vsize, 'rsize': rsize, 'raddr': raddr})
    return {'image_base': image_base, 'sections': sections, 'dirs': dirs}


def off_to_rva(pe, off):
    for s in pe['sections']:
        if s['raddr'] != 0 and s['raddr'] <= off < s['raddr'] + s['rsize']:
            return s['vaddr'] + (off - s['raddr']), s['name']
    return None, None


def rva_to_off(pe, rva):
    for s in pe['sections']:
        if s['vaddr'] <= rva < s['vaddr'] + max(s['vsize'], s['rsize']):
            off = s['raddr'] + (rva - s['vaddr'])
            if off < s['raddr'] + s['rsize']:
                return off, s['name']
            return None, s['name']  # virtual-only tail (.bss)
    return None, None


def diff_sections(orig_data, orig_pe, new_data, new_pe):
    """Diff every section present in the original exe; returns diff regions by RVA."""
    regions = []
    new_secs = {s['name']: s for s in new_pe['sections']}
    for s in orig_pe['sections']:
        if s['rsize'] == 0 or s['name'] not in new_secs:
            continue
        ns = new_secs[s['name']]
        n = min(s['rsize'], ns['rsize'])
        a = orig_data[s['raddr']:s['raddr'] + n]
        b = new_data[ns['raddr']:ns['raddr'] + n]
        diffs = [i for i in range(n) if a[i] != b[i]]
        i = 0
        while i < len(diffs):
            j = i
            while j + 1 < len(diffs) and diffs[j + 1] - diffs[j] <= DIFF_GAP_MERGE:
                j += 1
            lo, hi = diffs[i], diffs[j]
            regions.append({
                'rva': s['vaddr'] + lo,
                'len': hi - lo + 1,
                'sec': s['name'],
                'orig': a[lo:hi + 1],
                'new': b[lo:hi + 1],
            })
            i = j + 1
    regions.sort(key=lambda r: r['rva'])
    return regions


# ---------------------------------------------------------------- hook parsing

DIS_RE = re.compile(r'^(\s*)(//)?\s*(JMP|CALL)\s+(\S+)\s+([0-9A-Fa-f]+)(?:\s+(\d+))?\s*(.*)$')
MP_RE = re.compile(r'^(//)?\s*([01])\s+(\d+)\s+([0-9A-Fa-f]+)\s*(.*)$')
DEF_RE = re.compile(r'^\s*(?:;\s*)?(\w+)\s+@(\d+)\s*$')
XCK_RE = re.compile(r'^([0-9A-Fa-f]{8}):\s+([0-9A-Fa-f]{2})\s+([0-9A-Fa-f]{2})\s*$')


def parse_server_dis(path):
    hooks = []
    with open(path, 'r', encoding='latin1') as f:
        for lineno, line in enumerate(f, 1):
            m = DIS_RE.match(line.rstrip('\n'))
            if not m:
                continue
            _ind, dis, kind, export, va, extra, comment = m.groups()
            hooks.append({
                'va': int(va, 16),
                'kind': kind.lower(),
                'export': export,
                'extra': int(extra) if extra else 0,
                'comment': comment.strip().lstrip('/').strip(),
                'enabled': dis is None,
                'src': f'server.dis:{lineno}',
            })
    return hooks


def parse_client_mp(path, exports):
    hooks = []
    with open(path, 'r', encoding='latin1') as f:
        for lineno, line in enumerate(f, 1):
            m = MP_RE.match(line.rstrip('\n'))
            if not m:
                continue
            dis, kind_digit, ordinal, va, comment = m.groups()
            name = exports.get(int(ordinal), f'ord{ordinal}')
            hooks.append({
                'va': int(va, 16),
                'kind': 'jmp' if kind_digit == '0' else 'call',
                'export': name,
                'ord': int(ordinal),
                'extra': 0,
                'comment': comment.strip().lstrip('/').strip(),
                'enabled': dis is None,
                'src': f'allods2.mp:{lineno}',
            })
    return hooks


def parse_def(path):
    exports = {}
    if not os.path.exists(path):
        return exports
    with open(path, 'r', encoding='latin1') as f:
        for line in f:
            m = DEF_RE.match(line)
            if m:
                exports[int(m.group(2))] = m.group(1)
    return exports


def read_list(path):
    if not os.path.exists(path):
        return []
    out = []
    with open(path, 'r', encoding='latin1') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith((';', '#')):
                out.append(line)
    return out


def parse_xck(path):
    """Returns (entries[(off, a, b)], parse_error)."""
    entries = []
    try:
        with open(path, 'r', encoding='latin1') as f:
            for line in f:
                m = XCK_RE.match(line.strip())
                if m:
                    entries.append((int(m.group(1), 16), m.group(2).upper(), m.group(3).upper()))
    except OSError as e:
        return entries, str(e)
    return entries, None


def analyze_xcks(patches_dir, active_names, orig_data, orig_pe):
    """Parse every patch file; validate columns against the original exe bytes."""
    files = {}
    for fn in sorted(os.listdir(patches_dir)):
        path = os.path.join(patches_dir, fn)
        if not os.path.isfile(path):
            continue
        entries, err = parse_xck(path)
        if err:
            files[fn] = {'parsed': False, 'error': err, 'active': fn.lower() in active_names}
            continue
        col1_orig = col2_orig = 0
        vas = []
        for off, a, b in entries:
            rva, sec = off_to_rva(orig_pe, off)
            vas.append((off, rva, sec))
            actual = f'{orig_data[off]:02X}' if off < len(orig_data) else '??'
            if actual == a:
                col1_orig += 1
            if actual == b:
                col2_orig += 1
        n = len(entries)
        verdict = 'offset:before->after' if n and col1_orig >= col2_orig and col1_orig >= n * 0.9 else \
                  'offset:after->before' if n and col2_orig > col1_orig and col2_orig >= n * 0.9 else \
                  'UNCLEAR' if n else 'empty'
        files[fn] = {
            'parsed': True, 'entries': len(entries), 'verdict': verdict,
            'active': fn.lower() in active_names,
            'va_first': (vas[0][1] + orig_pe['image_base']) if vas and vas[0][1] is not None else None,
            'va_last': (vas[-1][1] + orig_pe['image_base'] + 1) if vas and vas[-1][1] is not None else None,
        }
    return files


# ------------------------------------------------------------ function extents

class FuncIndex:
    def __init__(self, snapshot_path):
        with open(snapshot_path, 'r', encoding='utf-8') as f:
            snap = json.load(f)
        self.funcs = sorted(
            ({'va': fn['va'],
              'end': fn['va'] + fn['size'],
              'name': '::'.join(fn['ns'] + [fn['name']])}
             for fn in snap['functions']),
            key=lambda x: x['va'])
        self._starts = [f['va'] for f in self.funcs]

    def find(self, va):
        """Function containing va (body), or None."""
        import bisect
        i = bisect.bisect_right(self._starts, va) - 1
        while i >= 0:
            f = self.funcs[i]
            if f['va'] <= va < f['end']:
                return f
            if f['end'] <= va and (i + 1 >= len(self.funcs) or self.funcs[i + 1]['va'] > va):
                return None
            if f['va'] <= va:
                return None
            i -= 1
        return None

    def at_entry(self, va):
        import bisect
        i = bisect.bisect_left(self._starts, va)
        if i < len(self.funcs) and self.funcs[i]['va'] == va:
            return self.funcs[i]
        return None


# ------------------------------------------------------------- repo VA lookup

def build_repo_refs():
    """va -> [(relpath:lineno), ...] from `// <va>` and `(client: <va>)` comments in src/."""
    refs = {}
    pat_addr = re.compile(r'//\s*\b([0-9A-Fa-f]{6})\b')
    pat_client = re.compile(r'\(\s*client\s*:\s*([0-9A-Fa-f]{5,6})\s*\)')
    src_dir = os.path.join(REPO, 'src')
    for root, _dirs, files in os.walk(src_dir):
        for fn in files:
            if not fn.endswith(('.h', '.cpp')):
                continue
            path = os.path.join(root, fn)
            rel = os.path.relpath(path, REPO).replace('\\', '/')
            try:
                with open(path, 'r', encoding='utf-8', errors='replace') as f:
                    for lineno, line in enumerate(f, 1):
                        for m in pat_addr.finditer(line):
                            refs.setdefault(int(m.group(1), 16), []).append(f'{rel}:{lineno}')
                        for m in pat_client.finditer(line):
                            refs.setdefault(int(m.group(1), 16), []).append(f'{rel}:{lineno} (client:)')
            except OSError:
                pass
    return refs


# ------------------------------------------------------------------- decoding

def decode(blob, base_va):
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        return None
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    out = []
    pos = 0
    while pos < len(blob):
        got = None
        for insn in md.disasm(blob[pos:], base_va + pos, count=1):
            got = insn
            break
        if got is None:
            out.append(f'{base_va + pos:X}: db {blob[pos]:02X}')
            pos += 1
        else:
            out.append(f'{got.address:X}: {got.mnemonic} {got.op_str}'.rstrip())
            pos += got.size
    return '; '.join(out)


# ---------------------------------------------------------------- inventory

def build_binary(name, cfg, do_decode):
    orig_data = open(cfg['orig'], 'rb').read()
    new_data = open(cfg['patched'], 'rb').read()
    orig_pe = parse_pe(orig_data)
    new_pe = parse_pe(new_data)
    image_base = orig_pe['image_base']

    regions = diff_regions = diff_sections(orig_data, orig_pe, new_data, new_pe)
    funcs = FuncIndex(cfg['snapshot'])

    # hooks
    if cfg['mp']:
        exports = parse_def(cfg['deffile'])
        hooks = parse_client_mp(cfg['dis'], exports)
    else:
        hooks = parse_server_dis(cfg['dis'])

    # xck files
    active_names = set(n.lower() for n in read_list(cfg['xck_lists'][0]))
    active_names |= set(n.lower() for n in read_list(cfg['xck_lists'][1]))
    if cfg['xck_glob']:
        for fn in os.listdir(cfg['patches_dir']):
            if fnmatch.fnmatch(fn.lower(), cfg['xck_glob']):
                active_names.add(fn.lower())
    xcks = analyze_xcks(cfg['patches_dir'], active_names, orig_data, orig_pe)

    # attach regions to hooks / functions, classify
    # NOTE: hook VAs (server.dis/allods2.mp) and snapshot VAs are FULL virtual
    # addresses (image base included); diff regions are tracked in RVA and
    # exposed with a 'va' full-VA key.
    import bisect
    for r in regions:
        r['va'] = image_base + r['rva']
    reg_starts = [r['va'] for r in regions]

    def region_at(va):
        i = bisect.bisect_right(reg_starts, va) - 1
        if i >= 0 and regions[i]['va'] <= va < regions[i]['va'] + regions[i]['len']:
            return regions[i]
        return None

    def region_overlapping(va, length):
        """A hook's written bytes may partially equal the original bytes, making
        the diff region start late or split; match on overlap, not containment."""
        i = max(0, bisect.bisect_right(reg_starts, va) - 8)
        while i < len(regions):
            r = regions[i]
            if r['va'] >= va + length:
                return None
            if r['va'] + r['len'] > va:
                return r
            i += 1
        return None

    takeovers = []
    known = KNOWN_TAKEOVERS.get(name, {})
    for h in hooks:
        if not h['enabled']:
            continue
        f = funcs.at_entry(h['va'])
        if f and h['kind'] == 'jmp':
            h['entry_hook'] = f['name']
            if h['export'] in known:
                h['takeover_of'] = f['name']
                takeovers.append({**h, 'func': f})

    def cave_owner(va):
        for t in takeovers:
            if t['va'] < va < t['func']['end']:
                return t
        return None

    def region_decoded(r):
        """Decode region bytes with a few trailing original bytes as context
        (a diff region may end mid-instruction, e.g. when the last hook byte
        happens to equal the original byte)."""
        ctx = b''
        off_end, _sec = rva_to_off(orig_pe, r['rva'] + r['len'])
        if off_end is not None:
            ctx = orig_data[off_end:off_end + 8]
        return decode(r['orig'] + ctx, r['va']), decode(r['new'] + ctx, r['va'])

    sites = []
    for h in hooks:
        s = {'type': 'hook', **h}
        s['va_full'] = h['va']
        s['func'] = funcs.find(h['va'])
        reg = region_overlapping(h['va'], 6 + h['extra'])
        s['region'] = {'va': reg['va'], 'len': reg['len'], 'sec': reg['sec']} if reg else None
        s['in_release'] = reg is not None
        owner = cave_owner(h['va'])
        s['cave_of'] = owner['export'] if owner else None
        if s['enabled'] and not s['in_release']:
            s['stale'] = True  # enabled hook whose bytes are not in the release exe
        if do_decode and reg is not None:
            _, s['new_decoded'] = region_decoded(reg)
        sites.append(s)

    import_rva, import_size = orig_pe['dirs'][1] if len(orig_pe['dirs']) > 1 else (0, 0)
    for r in regions:
        if import_rva and import_rva <= r['rva'] < import_rva + import_size:
            kind = 'import-area (injector)'
        else:
            kind = 'region'
        covered = any(lo <= r['va'] < lo + ln for lo, ln
                      in [(h2['va'], 5 + h2['extra']) for h2 in hooks if h2['enabled']])
        # skip a region line that is fully explained by the hook at the same VA
        if covered and any(h2['va'] == r['va'] and r['va'] + r['len'] <= h2['va'] + 6 + h2['extra']
                           for h2 in hooks if h2['enabled']):
            continue
        s = {'type': kind, 'rva': r['rva'], 'va_full': r['va'], 'len': r['len'],
             'sec': r['sec'], 'func': funcs.find(r['va']), 'covered_by_hook': covered}
        owner = cave_owner(r['va'])
        s['cave_of'] = owner['export'] if owner else None
        if do_decode:
            s['orig_decoded'], s['new_decoded'] = region_decoded(r)
        sites.append(s)

    for rt in RUNTIME_PATCHES.get(name, []):
        sites.append({'type': 'runtime-memcpy', 'va_full': rt['va'],
                      'what': rt['what'], 'func': funcs.find(rt['va'])})

    # group sites by owning function
    groups = {}
    for s in sites:
        key = None
        if s.get('func'):
            key = (s['func']['va'], s['func']['name'])
        groups.setdefault(key, []).append(s)

    return {
        'name': name,
        'orig': cfg['orig'], 'patched': cfg['patched'],
        'image_base': image_base,
        'regions': [{'va': r['va'], 'len': r['len'], 'sec': r['sec']} for r in regions],
        'hooks': hooks,
        'xcks': xcks,
        'sites': sites,
        'takeovers': [{'va': t['va'], 'export': t['export'],
                       'func': t['func']['name'], 'func_end': t['func']['end'],
                       'comment': t['comment'], 'evidence': known.get(t['export'], '')}
                      for t in takeovers],
        'groups': {str(k): v for k, v in groups.items()},
    }


def summarize(inv):
    lines = []
    active_hooks = [s for s in inv['sites'] if s['type'] == 'hook' and s['enabled']]
    disabled = [s for s in inv['sites'] if s['type'] == 'hook' and not s['enabled']]
    stale = [s for s in active_hooks if s.get('stale')]
    entry_hooks = [s for s in active_hooks if s.get('entry_hook')]
    cave = [s for s in inv['sites'] if s.get('cave_of')]
    regions = [s for s in inv['sites'] if s['type'] in ('region', 'import-area (injector)')]
    orphan = [s for s in regions if not s.get('covered_by_hook') and s['type'] == 'region']
    xck_files = inv['xcks']
    xck_active = [f for f, v in xck_files.items() if v.get('active')]
    xck_unclear = [f for f, v in xck_files.items() if v.get('parsed') and v.get('verdict') == 'UNCLEAR']
    lines.append(f"  hooks: {len(active_hooks)} active / {len(disabled)} disabled"
                 f" ({len(stale)} stale: enabled but not in release exe)")
    lines.append(f"  hooks at function entries: {len(entry_hooks)}"
                 f" (of which true takeovers with dead bodies: {len(inv['takeovers'])})")
    lines.append(f"  cave sites inside dead takeover bodies: {len(cave)}")
    lines.append(f"  diff regions: {len(regions)} ({len(orphan)} not covered by a hook)")
    lines.append(f"  xck files: {len(xck_files)} total, {len(xck_active)} active, {len(xck_unclear)} with unclear columns")
    unresolved = [s for s in inv['sites'] if s.get('va_full') and not s.get('func') and s['type'] != 'import-area (injector)']
    lines.append(f"  sites outside any known function: {len(unresolved)}")
    return lines, {'active_hooks': len(active_hooks), 'stale': len(stale), 'cave': len(cave),
                   'regions': len(regions), 'takeovers': len(inv['takeovers'])}


def fmt_site(s, ib):
    va = s.get('va_full', s.get('va'))
    parts = []
    if s['type'] == 'hook':
        state = '' if s['enabled'] else ' [DISABLED]'
        state += ' [STALE]' if s.get('stale') else ''
        if s.get('takeover_of'):
            state += f" **[TAKEOVER: {s['takeover_of']} replaced]**"
        elif s.get('entry_hook'):
            state += ' [entry-hook]'
        extra = f" +{s['extra']}N" if s.get('extra') else ''
        parts.append(f"- **hook** `{va:X}` {s['kind'].upper()} `{s['export']}`{extra}{state}"
                     f"  — {s['comment'] or ''} ({s['src']})")
    elif s['type'] == 'runtime-memcpy':
        parts.append(f"- **runtime** `{va:X}` {s['what']}")
    else:
        tag = 'region' if s['type'] == 'region' else 'region(injector)'
        cov = 'hook-covered' if s.get('covered_by_hook') else 'XCK-ONLY' if s['type'] == 'region' else ''
        parts.append(f"- **{tag}** `{va:X}` len={s['len']} [{s['sec']}] {cov}")
    if s.get('cave_of'):
        parts[0] += f"  **[cave: inside dead body of {s['cave_of']}]**"
    if s.get('func'):
        parts[0] += f"  fn: `{s['func']['name']}`"
    if s.get('orig_decoded') and s['type'] == 'region':
        parts.append(f"  - orig: `{s['orig_decoded']}`")
        parts.append(f"  - new:  `{s['new_decoded']}`")
    elif s.get('new_decoded') and s['type'] == 'hook':
        parts.append(f"  - bytes: `{s['new_decoded']}`")
    return '\n'.join(parts)


def write_md(invs, path, stats):
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('# Tangar/manager patch inventory (generated)\n\n')
        f.write('Generated by `tool/patch_inventory.py`. Ground truth = byte diff of the\n')
        f.write('pipeline outputs vs the original exes; hook/xck metadata from the patch\n')
        f.write('scripts. Runtime-only memcpys inside the DLLs are listed from source grep.\n\n')
        for inv, st in zip(invs, stats):
            f.write(f"## {inv['name']} — {inv['orig']} -> {inv['patched']}\n\n")
            for line in st:
                f.write(line + '\n')
            f.write('\n### True takeovers (function body dead, reimplemented in the DLL)\n\n')
            for t in inv['takeovers']:
                f.write(f"- `{t['va']:X}` `{t['func']}` (body ..{t['func_end']:X}) -> `{t['export']}`\n")
                if t['evidence']:
                    f.write(f"  - evidence: {t['evidence']}\n")
            f.write('\n### Sites by function\n\n')
            groups = sorted(inv['groups'].items(),
                            key=lambda kv: (kv[0] == 'None', -len(kv[1]), kv[0]))
            for key, sites in groups:
                if key == 'None':
                    continue
                va, fname = eval(key)
                takeover = any(s.get('takeover_of') for s in sites)
                f.write(f"#### `{fname}` @ {va:X}"
                        f"{' **[TAKEOVER]**' if takeover else ''}\n\n")
                for s in sorted(sites, key=lambda x: x.get('va_full', x.get('va', 0))):
                    f.write(fmt_site(s, inv['image_base']) + '\n')
                f.write('\n')
            none_sites = inv['groups'].get('None', [])
            if none_sites:
                f.write('#### (outside any function)\n\n')
                for s in sorted(none_sites, key=lambda x: x.get('va_full', x.get('va', 0))):
                    f.write(fmt_site(s, inv['image_base']) + '\n')
                f.write('\n')
            f.write('### xck patch files\n\n')
            f.write('| file | active | entries | column verdict | first VA | last VA |\n')
            f.write('|------|--------|---------|----------------|----------|---------|\n')
            for fn, v in sorted(inv['xcks'].items()):
                if not v.get('parsed'):
                    f.write(f"| {fn} | {v['active']} | ERR | {v.get('error', '')} | | |\n")
                else:
                    f.write(f"| {fn} | {v['active']} | {v['entries']} | {v['verdict']} | "
                            f"{(hex(v['va_first']) if v['va_first'] is not None else '')} | "
                            f"{(hex(v['va_last']) if v['va_last'] is not None else '')} |\n")
            f.write('\n')
    print(f'wrote {path}')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--no-decode', action='store_true')
    args = ap.parse_args()

    binaries = [
        ('server', {
            'orig': os.path.join(SRVMGR, 'postbuild', 'original', 'a2server.exe'),
            'patched': os.path.join(SRVMGR, 'postbuild', 'release', 'a2serv.exe'),
            'dis': os.path.join(SRVMGR, 'postbuild', 'server.dis'),
            'deffile': os.path.join(SRVMGR, 'srvmgr.def'),
            'mp': False,
            'patches_dir': os.path.join(SRVMGR, 'postbuild', 'patches'),
            'xck_lists': (os.path.join(SRVMGR, 'postbuild', 'arena.txt'),
                          os.path.join(SRVMGR, 'postbuild', 'srvlist.txt')),
            'xck_glob': 'bug_0*.xck',
            'snapshot': os.path.join(REPO, 'build', 'client_diff_snapshot_a2serv6_exe.json'),
        }),
        ('client', {
            'orig': os.path.join(A2MGR, 'postbuild', 'original', 'allods2.exe'),
            'patched': os.path.join(A2MGR, 'postbuild', 'release', 'allods2.exe'),
            'dis': os.path.join(A2MGR, 'postbuild', 'allods2.mp'),
            'deffile': os.path.join(A2MGR, 'a2mgr.def'),
            'mp': True,
            'patches_dir': os.path.join(A2MGR, 'postbuild', 'patches'),
            'xck_lists': (os.path.join(A2MGR, 'postbuild', 'list.txt'), ''),
            'xck_glob': None,
            'snapshot': os.path.join(REPO, 'build', 'client_diff_snapshot_allods2_exe.json'),
        }),
    ]

    invs, all_stats = [], []
    for name, cfg in binaries:
        print(f'[{name}] diffing {os.path.basename(cfg["orig"])} vs {os.path.basename(cfg["patched"])} ...')
        inv = build_binary(name, cfg, do_decode=not args.no_decode)
        invs.append(inv)
        lines, st = summarize(inv)
        print(f'[{name}]')
        for line in lines:
            print(line)
        all_stats.append(lines)

    out_json = os.path.join(REPO, 'migration_state', 'patch_inventory.json')
    with open(out_json, 'w', encoding='utf-8', newline='\n') as f:
        json.dump(invs, f, indent=1, default=lambda o: o.hex() if isinstance(o, (bytes, bytearray)) else str(o))
    print(f'wrote {out_json}')
    write_md(invs, os.path.join(REPO, 'migration_state', 'patch_inventory.md'), all_stats)


if __name__ == '__main__':
    main()
