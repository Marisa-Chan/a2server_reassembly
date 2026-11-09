import struct, sys

MAGIC = 0x31415926

def parse(path):
    f = open(path, "rb")
    magic, root_off, root_size, root_flags, fat_off, fat_size = struct.unpack("<6I", f.read(24))
    assert magic == MAGIC, hex(magic)
    if (root_flags & 0x80000000) == 0:
        f.seek(fat_off)
    fat = []
    for i in range(fat_size):
        m, off, size, flags = struct.unpack("<4I", f.read(16))
        name = f.read(16).split(b"\0")[0].decode("latin1")
        fat.append((m, off, size, flags, name))
    return f, fat, (root_off, root_size, root_flags)

def children(fat, node):
    if len(node) == 5:
        _, off, size, flags, name = node
    else:
        off, size, flags = node[0], node[1], node[2]
    return fat[off:off + size]

def walk(path_in, fpath):
    f, fat, root = parse(path_in)
    comps = [c for c in fpath.replace("/", "\\").split("\\") if c]
    # first component must match root name == archive file stem; skip it
    comps = comps[1:]
    nodes = children(fat, root)
    for comp in comps:
        match = [n for n in nodes if n[4].lower() == comp.lower()]
        if not match:
            print("NOT FOUND:", comp, "among", [n[4] for n in nodes][:40]); sys.exit(1)
        node = match[0]
        nodes = children(fat, node)
    return f, fat, node, nodes

if __name__ == "__main__":
    res = sys.argv[1]
    fpath = sys.argv[2]
    f, fat, node, nodes = walk(res, fpath)
    m, off, size, flags, name = node
    print(f"entry {name!r}: off={off} size={size} flags={flags:#x}")
    if len(sys.argv) > 3:  # dump
        f.seek(off)
        open(sys.argv[3], "wb").write(f.read(size))
        print("dumped to", sys.argv[3])
    else:  # list children
        for n in nodes:
            print(f"  child {n[4]!r} off={n[1]} size={n[2]} flags={n[3]:#x}")
