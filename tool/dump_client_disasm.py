#!/usr/bin/env python
"""Offline disassembly dump of client functions from the raw allods2.exe.

No Ghidra needed: maps VA->file offset via the PE section table, disassembles
with capstone starting at each entry VA for the known function size (from
build/func_hashes_client.json), and prints a normalized listing
(mnemonic + operands, call/jmp targets resolved to absolute VAs).

Usage:
    python tool/dump_client_disasm.py <hex_va> [<hex_va> ...]
    python tool/dump_client_disasm.py --all-vis-differs   # every pending Vis* client va
"""
import argparse
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
EXE = r'C:\Users\ikacn\Desktop\stuff\a2\a2land\client\allods2.exe'

_queue = os.path.join(ROOT, 'migration_state', 'client_verify_queue.txt')


def vis_differs_client_vas():
    out = []
    with open(_queue) as f:
        for line in f:
            p = line.rstrip('\n').split('\t')
            if len(p) >= 5 and p[0] in ('DIFFERS', 'FLAGGED') and p[1].startswith('Vis'):
                if p[4] not in ('-', ''):
                    out.append(int(p[4], 16))
    return sorted(set(out))


def load_pe(path):
    data = open(path, 'rb').read()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    nsec = struct.unpack_from('<H', data, pe + 6)[0]
    opt_size = struct.unpack_from('<H', data, pe + 20)[0]
    base = struct.unpack_from('<I', data, pe + 24 + 28)[0]
    secs = []
    st = pe + 24 + opt_size
    for i in range(nsec):
        off = st + i * 40
        name = data[off:off + 8].rstrip(b'\0').decode()
        vsize, va, rsize, raw = struct.unpack_from('<IIII', data, off + 8)
        secs.append((name, base + va, vsize, raw, rsize))
    return data, base, secs


def va_to_off(secs, va):
    for name, vaddr, vsize, raw, rsize in secs:
        if vaddr <= va < vaddr + max(vsize, rsize):
            return raw + (va - vaddr)
    return None


def disasm(data, secs, entry, size):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = False
    off = va_to_off(secs, entry)
    if off is None:
        return ['<VA not mapped>']
    code = data[off:off + size]
    lines = []
    for ins in md.disasm(code, entry):
        tgt = ''
        if ins.mnemonic[0] in ('j', 'c') and ins.op_str.startswith('0x'):
            try:
                tgt = ' ; -> %x' % int(ins.op_str, 16)
            except ValueError:
                pass
        lines.append('%6x: %-8s %s%s' % (ins.address, ins.mnemonic, ins.op_str, tgt))
        if ins.mnemonic == 'int3':
            break
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('vas', nargs='*')
    ap.add_argument('--all-vis-differs', action='store_true')
    ap.add_argument('--exe', default=EXE)
    args = ap.parse_args()

    vas = [int(v, 16) for v in args.vas]
    if args.all_vis_differs:
        vas = vis_differs_client_vas()

    cli = {int(it['entry'], 16): it for it in json.load(
        open(os.path.join(ROOT, 'build', 'func_hashes_client.json')))}
    data, base, secs = load_pe(args.exe)
    print('image base %x' % base)
    for va in vas:
        info = cli.get(va)
        size = info['size'] if info else 256
        print('=' * 60)
        print('client %x  size=%s ninsn=%s' % (va, info and info['size'], info and info['ninsn']))
        for ln in disasm(data, secs, va, size + 16):
            print(ln)


if __name__ == '__main__':
    main()
