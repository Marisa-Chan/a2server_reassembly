"""Find all CObject-derived vtables in Main.asm (CArray, CList, CMap template
instantiations, etc.).

Strategy:
  1. Scan for `off_XXXXXX` labels whose first `dd` entry is
     `?GetRuntimeClass@CObject@@UBEPAUCRuntimeClass@@XZ`, followed by exactly
     two arbitrary `dd offset` entries, then `?AssertValid@CObject@@UBEXXZ`
     and `?Dump@CObject@@UBEXAAVCDumpContext@@@Z`.
  2. For each vtable found, collect all `mov dword ptr [reg], offset off_XXXXXX`
     assignments (i.e. constructor sites) and report the enclosing proc name.
  3. Classify each vtable as CArray / CList / CMap / Unrelated using:
       a. Constructor window (25 lines after assignment):
            - `mov dword ptr [...], 11h`      → CMap  (m_nHashTableSize = 17, always a literal)
            - `mov dword ptr [...], 0Ah`      → CList (m_nBlockSize = 10, default arg)
       b. Destructor chain (scalar deleting dtor → real dtor body):
            - real dtor calls `??3@YAXPAX@Z`  → CArray (deleting m_pData)
            - otherwise                        → Unrelated/Other

Usage:
    .\find_cobj_vtables.py                # default ../src/Main.asm
    .\find_cobj_vtables.py [asm_file]     # custom path
"""
import sys
import os
import re

ASM_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "Main.asm")

VTABLE_START_RE = re.compile(
    r'^(off_[0-9A-Fa-f]+)\s+dd\s+offset\s+'
    r'(\?GetRuntimeClass@CObject@@UBEPAUCRuntimeClass@@XZ)\b'
)
DD_OFFSET_RE  = re.compile(r'^\s+dd\s+offset\s+(\S+)')
DD_ASSERT_RE  = re.compile(r'^\s+dd\s+offset\s+\?AssertValid@CObject@@UBEXXZ\b')
DD_DUMP_RE    = re.compile(r'^\s+dd\s+offset\s+\?Dump@CObject@@UBEXAAVCDumpContext@@@Z\b')
MOV_VTABLE_RE = re.compile(r'\bmov\s+dword\s+ptr\s+\[[^\]]*\]\s*,\s*offset\s+(off_[0-9A-Fa-f]+)\b')
PROC_RE       = re.compile(r'^(\S+)\s+proc\b')
ENDP_RE       = re.compile(r'^\S+\s+endp\b')

# Constructor window heuristics (case-insensitive for hex suffix)
CMAP_MAGIC_RE  = re.compile(r'\bmov\s+dword\s+ptr\s+\[[^\]]*\],\s*11h\b',  re.IGNORECASE)
CLIST_MAGIC_RE = re.compile(r'\bmov\s+dword\s+ptr\s+\[[^\]]*\],\s*0[Aa]h\b', re.IGNORECASE)

# Destructor chain heuristics
CALL_SUB_RE   = re.compile(r'\bcall\s+((?:sub|loc)_[0-9A-Fa-f]+)\b')
HEAP_FREE_RE  = re.compile(
    r'\bcall\s+(?:'
    r'\?\?3@YAXPAX@Z'         # operator delete(void*)
    r'|\?\?_V@YAXPAX@Z'       # operator delete[](void*)
    r'|_free'
    r'|ds:free'
    r'|ds:_free'
    r')\b',
    re.IGNORECASE,
)


def label_addr(label: str) -> int:
    return int(label[4:], 16)


# ------------------------------------------------------------------
# Proc index: maps proc name → (start_0idx, end_0idx)
# ------------------------------------------------------------------

def build_proc_index(lines: list[str]) -> dict[str, tuple[int, int]]:
    index: dict[str, tuple[int, int]] = {}
    current: str | None = None
    start = 0
    for i, line in enumerate(lines):
        pm = PROC_RE.match(line)
        if pm:
            current = pm.group(1)
            start = i
        elif ENDP_RE.match(line) and current is not None:
            index[current] = (start, i)
            current = None
    return index


# ------------------------------------------------------------------
# Classification
# ------------------------------------------------------------------

# How many lines after the vtable assignment to scan for magic constants.
# CMap's 11h appears at line+4 (field 2, 2 lines per field × 2 fields).
# CList's 0Ah appears at line+12 (field 6, 2 lines per field × 6 fields).
# The next member's 11h (in a tightly packed constructor like QuestMap) first
# appears at line+23, so capping at 14 captures CList without cross-contamination.
CTOR_WINDOW = 14


def classify(vtable: dict, lines: list[str], proc_index: dict[str, tuple[int, int]]) -> str:
    """Return one of: 'CMap', 'CList', 'CArray', 'Unrelated/Other', 'Unknown'."""

    # ----- Phase 1: constructor window scan ---------------------------
    for ctor in vtable["ctors"]:
        ctor_0 = ctor["line"] - 1   # convert to 0-based
        window = lines[ctor_0 : ctor_0 + CTOR_WINDOW]

        has_11h = any(CMAP_MAGIC_RE.search(l) for l in window)
        has_0ah = any(CLIST_MAGIC_RE.search(l) for l in window)

        if has_11h:
            return "CMap"
        if has_0ah:
            return "CList"

    # ----- Phase 2: destructor chain analysis -------------------------
    # The vtable dtor entry is the scalar deleting destructor.
    # Its first inner call leads to the real destructor whose body we inspect.
    dtor_name = vtable["dtor"]
    dtor_range = proc_index.get(dtor_name)
    if dtor_range is None:
        return "Unknown"

    dtor_start, dtor_end = dtor_range

    # Find the first call to a sub_XXXXXX inside the scalar deleting dtor.
    real_dtor_name: str | None = None
    for i in range(dtor_start, dtor_end + 1):
        m = CALL_SUB_RE.search(lines[i])
        if m:
            real_dtor_name = m.group(1)
            break

    if real_dtor_name is None:
        # Dtor with no calls → trivial base-class dtor → unrelated.
        return "Unrelated/Other"

    rd_range = proc_index.get(real_dtor_name)
    if rd_range is None:
        return "Unknown"

    rd_start, rd_end = rd_range

    # CArray's real dtor calls operator delete on m_pData.
    for i in range(rd_start, rd_end + 1):
        if HEAP_FREE_RE.search(lines[i]):
            return "CArray"

    return "Unrelated/Other"


# ------------------------------------------------------------------
# Main
# ------------------------------------------------------------------

def main() -> None:
    asm_file = os.path.abspath(sys.argv[1] if len(sys.argv) >= 2 else ASM_PATH)
    if not os.path.isfile(asm_file):
        print(f"ERROR: File not found: {asm_file}")
        sys.exit(1)

    print(f"Reading {asm_file} ...")
    with open(asm_file, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    print(f"  {len(lines):,} lines loaded.\n")

    # ------------------------------------------------------------------
    # Pass 1: locate CObject vtables
    # ------------------------------------------------------------------
    vtables: dict[str, dict] = {}

    for i in range(len(lines) - 4):
        m = VTABLE_START_RE.match(lines[i])
        if not m:
            continue
        label = m.group(1)

        m1 = DD_OFFSET_RE.match(lines[i + 1])
        m2 = DD_OFFSET_RE.match(lines[i + 2])
        m3 = DD_ASSERT_RE.match(lines[i + 3])
        m4 = DD_DUMP_RE.match(lines[i + 4])

        if m1 and m2 and m3 and m4:
            vtables[label] = {
                "line":      i + 1,          # 1-based
                "dtor":      m1.group(1),
                "serialize": m2.group(1),
                "ctors":     [],             # filled in pass 2
            }

    print(f"Found {len(vtables)} CObject-derived vtable(s).\n")

    # ------------------------------------------------------------------
    # Pass 2: find constructor assignments and build proc index
    # ------------------------------------------------------------------
    current_proc: tuple[str, int] | None = None

    for i, line in enumerate(lines):
        pm = PROC_RE.match(line)
        if pm:
            current_proc = (pm.group(1), i + 1)

        mm = MOV_VTABLE_RE.search(line)
        if mm:
            lbl = mm.group(1)
            if lbl in vtables:
                vtables[lbl]["ctors"].append({
                    "line": i + 1,
                    "proc": current_proc,
                })

    # ------------------------------------------------------------------
    # Pass 3: build proc index for destructor chain analysis
    # ------------------------------------------------------------------
    proc_index = build_proc_index(lines)

    # ------------------------------------------------------------------
    # Report
    # ------------------------------------------------------------------
    sorted_labels = sorted(vtables.keys(), key=label_addr)

    counts: dict[str, int] = {}

    for label in sorted_labels:
        v = vtables[label]
        kind = classify(v, lines, proc_index)
        counts[kind] = counts.get(kind, 0) + 1

        if kind not in ("CMap", "CList", "CArray"):
            continue

        print(f"{label}  [{kind}]  (line {v['line']})")
        print(f"  dtor:      {v['dtor']}")
        print(f"  serialize: {v['serialize']}")

        ctors = v["ctors"]
        if ctors:
            seen: dict[str, int] = {}
            for c in ctors:
                proc_name = c["proc"][0] if c["proc"] else "<unknown>"
                if proc_name not in seen:
                    seen[proc_name] = c["line"]
            print(f"  constructors ({len(seen)} unique proc(s)):")
            for proc_name, ln in seen.items():
                print(f"    {proc_name}  @ line {ln}")
        else:
            print("  (no constructor assignments found)")
        print()

    print(f"Total: {len(vtables)} vtable(s)")
    for kind, n in sorted(counts.items()):
        print(f"  {kind}: {n}")


if __name__ == "__main__":
    main()
