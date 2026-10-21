# Runs INSIDE the Ghidra daemon on the CLIENT program.
# The client build emits per-class GetRuntimeClass stubs ('mov eax, &CRuntimeClass; ret')
# as vtable slot 0; Ghidra never defined them as functions, so raw vtable runs start one
# slot late. Create functions at the .text targets of the dwords ADJACENT to each run.

import json
import struct

import jpype

snap = json.load(open(r'C:\Users\ikacn\Desktop\stuff\a2\assembly\build\client_diff_snapshot_allods2_exe.json'))
fm = program.getFunctionManager()
af = program.getAddressFactory()
mem = program.getMemory()

sections = []
for b in mem.getBlocks():
    if b.getName() in ('.rdata', '.data') and b.isInitialized():
        n = int(b.getSize())
        jarr = jpype.JArray(jpype.JByte)(n)
        b.getBytes(b.getStart(), jarr)
        sections.append((int(str(b.getStart()), 16), bytes(jarr)))


def read_dword(va):
    for base, data in sections:
        if base <= va < base + len(data) - 3:
            return struct.unpack_from('<I', data, va - base)[0]
    return None


made = 0
from ghidra.program.model.address import AddressSet
from ghidra.program.model.symbol import SourceType

with ctx.transaction('define vtable stub functions'):
    for run in snap['vtables']:
        va = run['va']
        end = va + 4 * len(run['slots'])
        for cand in (va - 4, end):
            t = read_dword(cand)
            if not t or not (0x401000 <= t < 0x5d4e00):
                continue
            addr = af.getAddress(hex(t))
            if fm.getFunctionAt(addr) is None and fm.getFunctionContaining(addr) is None:
                body = AddressSet(addr, addr.add(5))  # mov eax, imm32 ; ret
                try:
                    if fm.createFunction(None, addr, body, SourceType.DEFAULT) is not None:
                        made += 1
                except Exception:
                    pass  # overlaps an existing function body; leave it
print('functions created:', made)
ctx.save('define vtable stub functions')
print('saved')
