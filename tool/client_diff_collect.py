# Runs INSIDE the Ghidra daemon via:  python tool/gh.py python -f tool/client_diff_collect.py
# (use MSYS_NO_PATHCONV=1 when starting the daemon with --program /allods2.exe)
#
# Collects per-program facts for the client/server correspondence work:
#   - every function: va, name, namespace chain, body size, thunk flag
#   - every vtable-shaped run in .rdata/.data (>=3 consecutive fn-entry dwords)
#   - calls:     fn va -> ordered list of direct-call targets (function entries)
#   - datarefs:  fn va -> ordered list of referenced data vas (strings, vtables)
#   - strings:   va -> ASCII content, extracted from the section dumps
# Writes build/client_diff_snapshot_<programname>.json. Pure read, no program edits.

import json
import struct

import jpype

OUT_TMPL = r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\client_diff_snapshot_%s.json'

fm = program.getFunctionManager()
mem = program.getMemory()

def try_hex(s):
    try:
        return int(s, 16)
    except ValueError:
        return None


funcs = []
for f in fm.getFunctions(True):
    ns = []
    p = f.getParentNamespace()
    while p is not None and not p.isGlobal():
        ns.insert(0, p.getName())
        p = p.getParentNamespace()
    thunk_to = None
    if f.isThunk():
        tf = f.getThunkedFunction(False)
        if tf is not None:
            thunk_to = try_hex(str(tf.getEntryPoint()))
    funcs.append({
        'va': int(str(f.getEntryPoint()), 16),
        'name': f.getName(),
        'ns': ns,
        'size': int(f.getBody().getNumAddresses()),
        'thunk': bool(f.isThunk()),
        'thunk_to': thunk_to,
    })
funcs.sort(key=lambda x: x['va'])
entries = {x['va'] for x in funcs}
starts = [x['va'] for x in funcs]
print('functions:', len(funcs))

text_block = None
sections = []
for b in mem.getBlocks():
    if b.getName() == '.text':
        t_start = int(str(b.getStart()), 16)
        t_end = t_start + int(b.getSize())
    elif b.getName() in ('.rdata', '.data') and b.isInitialized():
        n = int(b.getSize())
        jarr = jpype.JArray(jpype.JByte)(n)
        b.getBytes(b.getStart(), jarr)
        sections.append((int(str(b.getStart()), 16), bytes(jarr)))
        print('section', b.getName(), hex(sections[-1][0]), hex(len(sections[-1][1])))
print('text range', hex(t_start), hex(t_end))


def containing_fn(va):
    import bisect
    i = bisect.bisect_right(starts, va) - 1
    if i < 0:
        return None
    if va < starts[i] + max(funcs[i]['size'], 16):
        return starts[i]
    return None


# --- one ordered pass over all references from .text --------------------------
refMgr = program.getReferenceManager()
it = refMgr.getReferenceIterator(program.getAddressFactory().getAddress(hex(t_start)))
calls = {}
datarefs = {}
n = 0
while it.hasNext():
    r = it.next()
    from_va = try_hex(str(r.getFromAddress()))
    if from_va is None or from_va >= t_end:
        if from_va is None:
            continue
        break
    n += 1
    fn = containing_fn(from_va)
    if fn is None:
        continue
    rt = r.getReferenceType()
    to_addr = r.getToAddress()
    if to_addr is None:
        continue
    to_va = try_hex(str(to_addr))
    if to_va is None:
        continue
    if rt.isCall():
        if to_va in entries:
            calls.setdefault(fn, []).append(to_va)
    elif rt.isData():
        datarefs.setdefault(fn, []).append(to_va)
print('refs walked:', n, 'fns with calls:', len(calls), 'fns with datarefs:', len(datarefs))

# --- strings from the section dumps -------------------------------------------
strings = {}
for base, data in sections:
    i = 0
    n2 = len(data)
    while i < n2:
        c = data[i]
        if 32 <= c < 127:
            j = i
            while j < n2 and 32 <= data[j] < 127:
                j += 1
            if j - i >= 5:
                strings[base + i] = data[i:j].decode('ascii')
            i = j
        else:
            i += 1
print('strings >=5 chars:', len(strings))


# --- vtable-shaped runs --------------------------------------------------------
def section_of(va):
    for base, data in sections:
        if base <= va < base + len(data):
            return base, data
    return None


vtables = []
for base, data in sections:
    cnt = len(data) // 4
    words = struct.unpack_from('<%dI' % cnt, data, 0)
    i = 0
    while i < cnt:
        va = base + i * 4
        if words[i] not in entries:
            i += 1
            continue
        j = i
        while j < cnt and words[j] in entries:
            j += 1
        if j - i >= 3 and va - 4 >= base:
            rtti_name = None
            col = words[i - 1]
            if col and (col & 3) == 0 and section_of(col) is not None:
                def rd(va2):
                    sec = section_of(va2)
                    if sec is None:
                        return None
                    b2, d2 = sec
                    if va2 + 4 <= b2 + len(d2):
                        return struct.unpack_from('<I', d2, va2 - b2)[0]
                    return None
                if rd(col) == 0:
                    td = rd(col + 12)
                    if td is not None and section_of(td) is not None:
                        b2, d2 = section_of(td)
                        off = td + 8 - b2
                        end = d2.find(b'\0', off, min(off + 256, len(d2)))
                        if end > off:
                            try:
                                cand = d2[off:end].decode('ascii')
                                if cand.startswith('.?A'):
                                    rtti_name = cand
                            except UnicodeDecodeError:
                                pass
            vtables.append({
                'rtti': rtti_name,
                'va': va,
                'slots': [words[k] for k in range(i, j)],
            })
        i = j

print('vtable runs:', len(vtables))
out = {'program': program.getName(), 'functions': funcs, 'vtables': vtables,
       'calls': {('%x' % k): v for k, v in calls.items()},
       'datarefs': {('%x' % k): v for k, v in datarefs.items()},
       'strings': {('%x' % k): v for k, v in strings.items()}}
path = OUT_TMPL % program.getName().replace('.', '_')
with open(path, 'w') as fh:
    json.dump(out, fh)
print('wrote', path)
