"""Scan Main.asm for unused functions and data symbols via static analysis.

Strategy: extract all identifier-like tokens from each line, look them up in a
set of known symbols.  This is O(total_tokens) instead of O(symbols * lines).

Usage:
    python asm_unused.py                       # report only
    python asm_unused.py --cleanup=functions    # delete unused functions
    python asm_unused.py --cleanup=data         # delete unused data symbols
    python asm_unused.py --cleanup=all          # delete both
    python asm_unused.py --cleanup=all --dry-run  # preview without writing
"""
import re
import sys
import os
import glob
import argparse
from collections import defaultdict

ASM_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "Main.asm")

# Matches most IDA symbol names: sub_XXX, loc_XXX, off_XXX, ?Mangled@@..., etc.
TOKEN_RE = re.compile(r"[A-Za-z_?@$][A-Za-z0-9_?@$]*")
PROC_RE = re.compile(r"^(\S+)\s+proc\s+near")
ENDP_RE = re.compile(r"^(\S+)\s+endp\b")
DATA_RE = re.compile(r"^((?:dword|word|byte|qword|dd|dw|db|off|stru|unk|flt|dbl)_[0-9A-Fa-f]+)\s")
SEPARATOR_RE = re.compile(r"^;\s*-{10,}")
LABEL_RE = re.compile(r"^[A-Za-z_?@$][A-Za-z0-9_?@$]*\s")
# Unlabeled data continuation: lines starting with whitespace then db/dd/dw/dq etc.
DATA_CONT_RE = re.compile(r"^\s+(db|dd|dw|dq)\s")


def scan_references(lines, procs, data_syms, asm_path):
    """Return ref_counts: symbol_name -> number of lines referencing it."""
    all_names = set(procs.keys()) | set(data_syms.keys())

    proc_def_lines = set()
    for i, line in enumerate(lines):
        if PROC_RE.match(line) or ENDP_RE.match(line):
            proc_def_lines.add(i)

    data_def_lines = {}
    for i, line in enumerate(lines):
        m = DATA_RE.match(line)
        if m:
            data_def_lines[i] = m.group(1)

    ref_counts = defaultdict(int)
    for i, line in enumerate(lines):
        if i in proc_def_lines:
            continue
        tokens = TOKEN_RE.findall(line)
        if not tokens:
            continue
        self_name = data_def_lines.get(i)
        seen = set()
        for tok in tokens:
            if tok in seen:
                continue
            seen.add(tok)
            if tok in all_names and tok != self_name:
                ref_counts[tok] += 1

    src_dir = os.path.dirname(asm_path)
    inc_files = glob.glob(os.path.join(src_dir, "*.inc"))
    for inc_path in inc_files:
        if os.path.abspath(inc_path) == os.path.abspath(asm_path):
            continue
        with open(inc_path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                for tok in TOKEN_RE.findall(line):
                    if tok in all_names:
                        ref_counts[tok] += 1

    return ref_counts


def collect_symbols(lines):
    """Return (procs, data_syms) dicts: name -> 1-based line number."""
    procs = {}
    data_syms = {}
    for i, line in enumerate(lines):
        m = PROC_RE.match(line)
        if m:
            procs[m.group(1)] = i + 1
        m = DATA_RE.match(line)
        if m:
            data_syms[m.group(1)] = i + 1
    return procs, data_syms


def find_proc_ranges(lines, proc_name, proc_line_1based):
    """Find all line ranges for a function: proc body, chunks, SEH .rdata, jump tables.
    Returns list of (start, end) 1-based inclusive ranges, sorted ascending."""
    total = len(lines)
    ranges = []

    # --- proc..endp body ---
    proc_idx = proc_line_1based - 1
    endp_line = None
    for i in range(proc_idx, total):
        if ENDP_RE.match(lines[i]):
            endp_line = i + 1
            break
    if endp_line is None:
        return []

    # Extend upward to include separator/attribute comments above proc
    start_line = proc_line_1based
    for j in range(proc_idx - 1, max(-1, proc_idx - 6), -1):
        stripped = lines[j].rstrip()
        if SEPARATOR_RE.match(stripped):
            start_line = j + 1
            break
        # Also absorb blank lines and "; Attributes:" lines between separator and proc
        if stripped == "" or stripped.startswith("; Attributes:") or stripped.startswith(";"):
            start_line = j + 1
        else:
            break

    # Check for jump table immediately after endp
    body_end = endp_line
    for i in range(endp_line, min(total, endp_line + 500)):
        line = lines[i].strip()
        if SEPARATOR_RE.match(line) or PROC_RE.match(lines[i]) or "FUNCTION CHUNK" in line:
            break
        if re.match(r"^dd\s+offset", line) or line == "" or re.match(r"^;\s*jumptable", line):
            body_end = i + 1
        else:
            break

    ranges.append((start_line, body_end))

    # --- FUNCTION CHUNK FOR ---
    escaped = re.escape(proc_name)
    for i in range(total):
        if re.search(rf"FUNCTION CHUNK FOR\s+{escaped}\b", lines[i]):
            chunk_start = i + 1
            chunk_end = chunk_start
            for j in range(i + 1, min(total, i + 500)):
                if re.search(rf"END OF FUNCTION CHUNK FOR\s+{escaped}\b", lines[j]):
                    chunk_end = j + 1
                    break

            # Include separator above
            cs = chunk_start
            for j in range(i - 1, max(-1, i - 5), -1):
                if SEPARATOR_RE.match(lines[j]):
                    cs = j + 1
                    break
            # Include separator below
            ce = chunk_end
            if ce < total and SEPARATOR_RE.match(lines[ce]):
                ce += 1

            ranges.append((cs, ce))

    # Merge overlapping/adjacent ranges
    ranges.sort()
    merged = [ranges[0]]
    for s, e in ranges[1:]:
        if s <= merged[-1][1] + 1:
            merged[-1] = (merged[-1][0], max(merged[-1][1], e))
        else:
            merged.append((s, e))

    return merged


def find_data_range(lines, data_name, data_line_1based):
    """Find the line range for a data symbol: label line + continuation lines.
    Returns (start, end) 1-based inclusive."""
    total = len(lines)
    idx = data_line_1based - 1

    end_idx = idx
    for j in range(idx + 1, min(total, idx + 10000)):
        stripped = lines[j].strip()
        # Continuation: whitespace-indented db/dd/dw/dq, or blank lines between them
        if DATA_CONT_RE.match(lines[j]):
            end_idx = j
        elif stripped == "":
            # Blank line might separate — peek ahead
            if j + 1 < total and DATA_CONT_RE.match(lines[j + 1]):
                end_idx = j  # include blank, continue
            else:
                break
        else:
            break

    return (data_line_1based, end_idx + 1)


def delete_ranges(lines, ranges):
    """Delete line ranges from lines list (bottom-up). Returns number of deleted lines.
    `ranges` is list of (start, end) 1-based inclusive, assumed non-overlapping."""
    ranges_sorted = sorted(ranges, key=lambda x: x[0], reverse=True)
    total_deleted = 0
    for s, e in ranges_sorted:
        si = s - 1
        del lines[si:e]
        total_deleted += e - s + 1
    return total_deleted


def report(procs, data_syms, ref_counts):
    unused_procs = {name: ln for name, ln in procs.items() if ref_counts[name] == 0}
    unused_data = {name: ln for name, ln in data_syms.items() if ref_counts[name] == 0}
    unused_sub = {n: ln for n, ln in unused_procs.items() if n.startswith("sub_")}
    unused_lib = {n: ln for n, ln in unused_procs.items() if not n.startswith("sub_")}

    print(f"\n=== FUNCTIONS ===")
    print(f"Total procs:              {len(procs)}")
    print(f"Unreferenced procs:       {len(unused_procs)}")
    print(f"  sub_XXXXX (game code):  {len(unused_sub)}")
    print(f"  Named/library/CRT:      {len(unused_lib)}")

    print(f"\n=== DATA SYMBOLS ===")
    print(f"Total data symbols:       {len(data_syms)}")
    print(f"Unreferenced data:        {len(unused_data)}")

    return unused_procs, unused_data


def main():
    parser = argparse.ArgumentParser(description="Find and optionally delete unused symbols in Main.asm")
    parser.add_argument("asm_file", nargs="?", default=ASM_PATH, help="Path to Main.asm")
    parser.add_argument("--cleanup", choices=["functions", "data", "all"], default=None,
                        help="Delete unused symbols: functions, data, or all")
    parser.add_argument("--dry-run", action="store_true",
                        help="Preview what would be deleted without writing")
    args = parser.parse_args()

    path = os.path.abspath(args.asm_file)
    print(f"Reading {path} ...")
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    print(f"  {len(lines)} lines loaded.")

    procs, data_syms = collect_symbols(lines)
    print(f"  {len(procs)} procs, {len(data_syms)} data symbols found.")

    print("  Scanning for references ...")
    ref_counts = scan_references(lines, procs, data_syms, path)

    unused_procs, unused_data = report(procs, data_syms, ref_counts)

    if args.cleanup is None:
        return

    # --- Cleanup mode ---
    do_funcs = args.cleanup in ("functions", "all")
    do_data = args.cleanup in ("data", "all")

    all_ranges = []

    if do_funcs:
        for name, ln in unused_procs.items():
            try:
                proc_ranges = find_proc_ranges(lines, name, ln)
                all_ranges.extend(proc_ranges)
            except Exception as e:
                print(f"  WARNING: Could not find ranges for {name}: {e}")

    if do_data:
        for name, ln in unused_data.items():
            try:
                data_range = find_data_range(lines, name, ln)
                all_ranges.append(data_range)
            except Exception as e:
                print(f"  WARNING: Could not find range for {name}: {e}")

    if not all_ranges:
        print("\nNothing to delete.")
        return

    # Sort and merge overlapping ranges
    all_ranges.sort()
    merged = [all_ranges[0]]
    for s, e in all_ranges[1:]:
        if s <= merged[-1][1] + 1:
            merged[-1] = (merged[-1][0], max(merged[-1][1], e))
        else:
            merged.append((s, e))

    count = sum(e - s + 1 for s, e in merged)
    n_funcs = len(unused_procs) if do_funcs else 0
    n_data = len(unused_data) if do_data else 0
    print(f"\n  {n_funcs} unused funcs, {n_data} unused data -> {len(merged)} ranges, {count} lines")

    total_deleted = 0
    if args.dry_run:
        for s, e in merged:
            first = lines[s - 1].rstrip()[:77]
            print(f"    {s}-{e} ({e - s + 1} lines) {first}")
        total_deleted = count
    else:
        total_deleted = delete_ranges(lines, merged)
        print(f"    Deleted {total_deleted} lines.")

    if args.dry_run:
        print(f"\nDRY RUN: Would delete ~{total_deleted} lines total.")
    else:
        with open(path, "w", encoding="utf-8") as f:
            f.writelines(lines)
        print(f"\nDone. Deleted {total_deleted} lines total. File now has {len(lines)} lines.")


if __name__ == "__main__":
    main()
