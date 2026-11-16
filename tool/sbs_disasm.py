#!/usr/bin/env python
# Side-by-side disasm of original vs patched a2server.exe at a VA (helper for
# the bug_00xx porting loop; not part of the build).
import sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

ORIG = r'C:/Users/ikacn/Desktop/stuff/a2/my-srvmgr/postbuild/original/a2server.exe'
NEW = r'C:/Users/ikacn/Desktop/stuff/a2/my-srvmgr/postbuild/release/a2serv.exe'


def fo(va):
    return va - 0x400C00


def main():
    va = int(sys.argv[1], 16)
    n = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x40
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    orig = open(ORIG, 'rb').read()
    new = open(NEW, 'rb').read()
    for tag, data in [('ORIG', orig), ('PATCHED', new)]:
        print(f'--- {tag} ---')
        for i in md.disasm(data[fo(va):fo(va) + n], va):
            print(f'{i.address:X} {i.mnemonic} {i.op_str}')


if __name__ == '__main__':
    main()
