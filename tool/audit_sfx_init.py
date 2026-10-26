"""Audit: SfxSample* fields passed to FreeSfxSample but never zero-initialized.

For every `FreeSfxSample(&this->FIELD...)` call site in src/*.cpp, find the
enclosing class function, then check whether FIELD is ever assigned nullptr
anywhere for that class (cpp methods or header NSDMI `= nullptr`).
Loops (FIELD[i]) are handled by matching the base name too.
"""
import re
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parent.parent / "src"

func_re = re.compile(r"^([\w:~<>, ]+[\w:>])::(~?\w+)\s*\(")
free_re = re.compile(r"(?:FreeSfxSample|ReloadSfx|StopSfx|PlayAmbientSfx)\(&this->(\w+)")

# collect (file, class, field)
sites = []
nulled = {}  # (class, base_field) -> True

for cpp in SRC.glob("*.cpp"):
    cur_cls = None
    for i, line in enumerate(cpp.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
        m = func_re.match(line)
        if m:
            cur_cls = m.group(1)
            continue
        for fm in free_re.finditer(line):
            fld = fm.group(1)
            sites.append((cpp.name, i, cur_cls, fld))
        # nullptr assignments to this->field (covers FIELD[i] and FIELD[i].member = nullptr)
        for am in re.finditer(r"this->(\w+)(\[[^\]]*\])?(\.\w+)?\s*=\s*nullptr", line):
            if cur_cls:
                nulled[(cur_cls, am.group(1))] = True
        # reload assigns the field afterwards, so a reload-only field is still
        # a first-use hazard via FreeSfxSample inside ReloadSfx -> keep flagged

# header NSDMI: SfxSample* field = nullptr;  (class-scoped)
hdr_nsdmi = {}  # field -> True (any class; coarse but fields are mostly unique)
hdr_class_fields = {}  # (class, field) -> True
cur_cls = None
for hdr in SRC.glob("*.h"):
    for line in hdr.read_text(encoding="utf-8", errors="replace").splitlines():
        cm = re.match(r"^class\s+(\w+)", line)
        if cm:
            cur_cls = cm.group(1)
        fm = re.match(r"\s*SfxSample\*\s*(\w+)(\[\d+\])?\s*(=\s*nullptr)?", line)
        if fm and cur_cls:
            hdr_class_fields[(cur_cls, fm.group(1))] = bool(fm.group(3))
            if fm.group(3):
                nulled[(cur_cls, fm.group(1))] = True

problems = {}
for fname, line, cls, fld in sites:
    if cls is None:
        continue
    ok = nulled.get((cls, fld), False)
    if not ok:
        problems.setdefault((cls, fld), []).append(f"{fname}:{line}")

print("=== FreeSfxSample on fields never null-initialized (class, field) ===")
for (cls, fld), locs in sorted(problems.items()):
    print(f"{cls}::{fld}  ({locs[0]}, {len(locs)} site(s))")
print(f"\ntotal: {len(problems)} field(s)")
