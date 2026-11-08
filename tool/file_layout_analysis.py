"""Analyze class-to-file layout: full class definitions per header vs method definitions per cpp."""
import os, re, collections

SRC = os.path.join(os.path.dirname(__file__), '..', 'src')

# Full definitions only: 'class Foo {' / 'class Foo : public Bar {' (not 'class Foo;')
class_def_re = re.compile(r'^(?:class|struct)\s+(\w+)[^;]*$', re.M)
def_re = re.compile(r'^\s*(?:[\w:<>,\s\*&]+?\s+)?(\w+)::(\w+)\s*[\(<]', re.M)

def strip_comment(line):
    # remove a trailing // comment so '... { //438ec0' reads as '... {'
    i = line.find('//')
    return line[:i] if i >= 0 else line

def is_definition(lines, i):
    """A definition's signature terminates in '{' before any ';' appears.
    Call statements (e.g. CSound::Play(x);) terminate in ';' first."""
    depth = 0
    for line in lines[i:i + 40]:
        s = strip_comment(line).rstrip()
        depth += s.count('(') - s.count(')')
        if depth < 0:
            depth = 0
        if s.endswith(';'):
            return False
        if s.endswith('{') and depth <= 0:
            return True
    return False

decl = {}   # class -> header (full definition, first wins)
for h in sorted(f for f in os.listdir(SRC) if f.endswith('.h')):
    txt = open(os.path.join(SRC, h), encoding='utf-8', errors='replace').read()
    for m in class_def_re.finditer(txt):
        decl.setdefault(m.group(1), h)

defs = collections.defaultdict(lambda: collections.Counter())
for c in sorted(f for f in os.listdir(SRC) if f.endswith('.cpp')):
    with open(os.path.join(SRC, c), encoding='utf-8', errors='replace') as fh:
        lines = fh.readlines()
    for i, line in enumerate(lines):
        m = def_re.match(line)
        if m and is_definition(lines, i):
            defs[m.group(1)][c] += 1

print("=" * 100)
print("REAL MISMATCHES: full class def in header X, majority of methods in cpp Y (stems differ)")
print("=" * 100)
rows = []
for cls in sorted(defs):
    h = decl.get(cls)
    if h is None:
        continue
    best_cpp, best_n = max(defs[cls].items(), key=lambda kv: kv[1])
    if best_cpp[:-4] != h[:-2]:
        rows.append((best_n, cls, h, dict(defs[cls])))
rows.sort(reverse=True)
for n, cls, h, parts in rows:
    ps = ", ".join(f"{c}:{k}" for c, k in sorted(parts.items(), key=lambda kv: -kv[1]))
    print(f"  {cls:38s} decl:{h:22s} defs: {ps}")

print()
print("=" * 100)
print("DEFINED IN CPP BUT NO FULL CLASS DEF IN ANY HEADER")
print("=" * 100)
for cls in sorted(defs):
    if cls not in decl:
        ps = ", ".join(f"{c}:{k}" for c, k in sorted(defs[cls].items(), key=lambda kv: -kv[1]))
        print(f"  {cls:38s} {ps}")

print()
print("=" * 100)
print("CLASSES DEFINED (header) BUT NEVER IMPLEMENTED (no :: in any cpp) — likely dead or inline-only")
print("=" * 100)
implemented = set(defs)
for cls, h in sorted(decl.items(), key=lambda t: (t[1], t[0])):
    if cls not in implemented:
        print(f"  {cls:38s} {h}")
