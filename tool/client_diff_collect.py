# Runs INSIDE the Ghidra daemon via:  python tool/gh.py python -f tool/client_diff_collect.py
# (use MSYS_NO_PATHCONV=1 when starting the daemon with --program /allods2.exe)
#
# Collects per-program facts for the client/server correspondence diff:
#   - every function: va, name, namespace chain, body size, thunk flag
#   - every RTTI vtable: class name, vtable va, slot function vas
# Writes build/client_diff_snapshot_<programname>.json. Pure read, no program edits.

import json
import struct

import jpype

OUT_TMPL = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\client_diff_snapshot_%s.json'

fm = program.getFunctionManager()
mem = program.getMemory()

funcs = []
for f in fm.getFunctions(True):
    ns = []
    p = f.getParentNamespace()
    while p is not None and not p.isGlobal():
        ns.insert(0, p.getName())
        p = p.getParentNamespace()
    funcs.append({
        'va': int(str(f.getEntryPoint()), 16),
        'name': f.getName(),
        'ns': ns,
        'size': int(f.getBody().getNumAddresses()),
        'thunk': bool(f.isThunk()),
    })
entries = {x['va'] for x in funcs}
print('functions:', len(funcs))

# bulk-dump .rdata + .data (vtables live in .rdata, RTTI type descriptors in .data)
sections = []
for b in mem.getBlocks():
    if b.getName() in ('.rdata', '.data') and b.isInitialized():
        n = int(b.getSize())
        jarr = jpype.JArray(jpype.JByte)(n)
        b.getBytes(b.getStart(), jarr)
        sections.append((int(str(b.getStart()), 16), bytes(jarr)))
        print('section', b.getName(), hex(sections[-1][0]), hex(len(sections[-1][1])))


def read_dword(va):
    for base, data in sections:
        if base <= va < base + len(data) - 3:
            return struct.unpack_from('<I', data, va - base)[0]
    return None


def read_cstr(va, maxlen=256):
    for base, data in sections:
        if base <= va < base + len(data):
            off = va - base
            end = data.find(b'\0', off, min(off + maxlen, len(data)))
            if end < 0:
                return None
            try:
                return data[off:end].decode('ascii')
            except UnicodeDecodeError:
                return None
    return None


def section_of(va):
    for base, data in sections:
        if base <= va < base + len(data):
            return base, data
    return None


# scan for vtable-shaped runs: >=3 consecutive dwords pointing at function entries.
# RTTI (COL pointer at [-1] + type descriptor name) is recorded when present, but the
# game code was built without RTTI (VC6 default) - only static MFC vtables carry it.
vtables = []
for base, data in sections:
    n = len(data) // 4
    words = struct.unpack_from('<%dI' % n, data, 0)
    i = 0
    while i < n:
        va = base + i * 4
        w = words[i]
        if w not in entries:
            i += 1
            continue
        j = i
        while j < n and words[j] in entries:
            j += 1
        run_len = j - i
        if run_len >= 3 and va - 4 >= base:
            rtti_name = None
            col = words[i - 1]
            if col and (col & 3) == 0 and section_of(col) is not None:
                sig = read_dword(col)
                if sig == 0:
                    td = read_dword(col + 12)
                    if td is not None:
                        cand = read_cstr(td + 8)
                        if cand and cand.startswith('.?A'):
                            rtti_name = cand
            vtables.append({
                'rtti': rtti_name,
                'va': va,
                'slots': [words[k] for k in range(i, j)],
            })
        i = j

print('rtti vtables:', len(vtables))
out = {'program': program.getName(), 'functions': funcs, 'vtables': vtables}
path = OUT_TMPL % program.getName().replace('.', '_')
with open(path, 'w') as fh:
    json.dump(out, fh)
print('wrote', path)
