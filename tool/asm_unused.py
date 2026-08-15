"""Scan Main.asm for unused functions and data symbols via static analysis.

Strategy: extract all identifier-like tokens from each line, look them up in a
set of known symbols.  This is O(total_tokens) instead of O(symbols * lines).

Usage:
    python tool/asm_unused.py                       # report only
    python tool/asm_unused.py --cleanup=functions    # delete unused functions
    python tool/asm_unused.py --cleanup=data         # delete unused data symbols
    python tool/asm_unused.py --cleanup=all          # delete both
    python tool/asm_unused.py --cleanup=all --dry-run  # preview without writing
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
                for tok in TOKEN_RE.findall(line.split(';')[0]):
                    if tok in all_names:
                        ref_counts[tok] += 1

    return ref_counts


def build_call_graph(lines, procs, asm_path):
    """Build call graph: caller_proc -> set of callee_procs.
    Returns (calls, external_refs) where external_refs are procs referenced
    from outside any proc body (e.g., .inc files, data, jump tables)."""
    all_proc_names = set(procs.keys())

    # Map each line to its owning proc, including function chunks.
    chunk_start_re = re.compile(r"FUNCTION CHUNK FOR\s+(\S+)")
    chunk_end_re = re.compile(r"END OF FUNCTION CHUNK FOR\s+(\S+)")
    chunk_owner = [None] * len(lines)
    i = 0
    while i < len(lines):
        m = chunk_start_re.search(lines[i])
        if m:
            name = m.group(1)
            chunk_owner[i] = name
            for j in range(i + 1, len(lines)):
                chunk_owner[j] = name
                if chunk_end_re.search(lines[j]):
                    i = j + 1
                    break
            else:
                break
        else:
            i += 1

    line_to_proc = [None] * len(lines)
    current_proc = None
    for i, line in enumerate(lines):
        m = PROC_RE.match(line)
        if m:
            current_proc = m.group(1)
        elif ENDP_RE.match(line):
            current_proc = None
        line_to_proc[i] = current_proc or chunk_owner[i]

    calls = defaultdict(set)

    for i, line in enumerate(lines):
        caller = line_to_proc[i]
        tokens = TOKEN_RE.findall(line)
        seen = set()
        for tok in tokens:
            if tok in seen:
                continue
            seen.add(tok)
            if tok not in all_proc_names:
                continue
            if caller is not None and tok != caller:
                calls[caller].add(tok)

    # External roots: procs referenced from .inc files (C++ interface / exports).
    external_refs = set()
    src_dir = os.path.dirname(asm_path)
    inc_files = glob.glob(os.path.join(src_dir, "*.inc"))
    for inc_path in inc_files:
        if os.path.abspath(inc_path) == os.path.abspath(asm_path):
            continue
        with open(inc_path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                for tok in TOKEN_RE.findall(line.split(';')[0]):
                    if tok in all_proc_names:
                        external_refs.add(tok)

    return calls, external_refs


def compute_reachable(calls, roots):
    reachable = set(roots)
    stack = list(roots)
    while stack:
        node = stack.pop()
        for callee in calls.get(node, set()):
            if callee not in reachable:
                reachable.add(callee)
                stack.append(callee)
    return reachable


def find_shortest_path_to_target(calls, roots, target):
    """Find one shortest path from any root to target using BFS.
    Returns a list of proc names from root to target, or None if unreachable."""
    if target in roots:
        return [target]

    # BFS forwards from roots, tracking parent for path reconstruction.
    parent = {}
    queue = list(roots)
    for root in roots:
        parent[root] = None
    seen = set(roots)

    while queue:
        node = queue.pop(0)
        for callee in calls.get(node, set()):
            if callee in seen:
                continue
            seen.add(callee)
            parent[callee] = node
            if callee == target:
                # Reconstruct path.
                path = [callee]
                while parent[path[0]] is not None:
                    path.insert(0, parent[path[0]])
                return path
            queue.append(callee)

    return None


def map_proc_endps(lines, procs):
    """Return dict: proc_name -> endp line (1-based)."""
    proc_lines = {ln: name for name, ln in procs.items()}
    endps = {}
    current_proc = None
    for i, line in enumerate(lines):
        if i + 1 in proc_lines:
            current_proc = proc_lines[i + 1]
        if ENDP_RE.match(line) and current_proc is not None:
            endps[current_proc] = i + 1
            current_proc = None
    return endps


def collect_chunks(lines):
    """Return dict: proc_name -> list of (start, end) 1-based inclusive chunk ranges."""
    chunks = defaultdict(list)
    total = len(lines)
    i = 0
    chunk_start_re = re.compile(r"FUNCTION CHUNK FOR\s+(\S+)")
    chunk_end_re = re.compile(r"END OF FUNCTION CHUNK FOR\s+(\S+)")
    while i < total:
        m = chunk_start_re.search(lines[i])
        if m:
            name = m.group(1)
            start = i + 1
            end = start
            for j in range(i + 1, min(total, i + 500)):
                if chunk_end_re.search(lines[j]):
                    end = j + 1
                    break
            chunks[name].append((start, end))
            i = end
        else:
            i += 1
    return chunks


def compute_reachable_sizes(procs, endps, chunks, reachable):
    total = 0
    reachable_total = 0
    for name in procs:
        endp = endps.get(name)
        if endp is None:
            continue
        size = endp - procs[name] + 1
        for s, e in chunks.get(name, []):
            size += e - s + 1
        total += size
        if name in reachable:
            reachable_total += size
    return total, reachable_total


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


def report(procs, data_syms, ref_counts, reachable=None, reachable_sizes=None, roots=None):
    unused_procs = {name: ln for name, ln in procs.items() if ref_counts[name] == 0}
    unused_data = {name: ln for name, ln in data_syms.items() if ref_counts[name] == 0}
    unused_sub = {n: ln for n, ln in unused_procs.items() if n.startswith("sub_")}
    unused_lib = {n: ln for n, ln in unused_procs.items() if not n.startswith("sub_")}

    print(f"\n=== FUNCTIONS ===")
    print(f"Total procs:              {len(procs)}")
    print(f"Unreferenced procs:       {len(unused_procs)}")
    print(f"  sub_XXXXX (game code):  {len(unused_sub)}")
    print(f"  Named/library/CRT:      {len(unused_lib)}")

    if reachable is not None and reachable_sizes is not None and roots is not None:
        total_lines, reachable_lines = reachable_sizes
        print(f"\n=== TRANSITIVE REACHABILITY ===")
        print(f"Root procs (external):    {len(roots)}")
        print(f"Reachable procs:          {len(reachable)} / {len(procs)}")
        print(f"Reachable lines:          {reachable_lines} / {total_lines}")

    print(f"\n=== DATA SYMBOLS ===")
    print(f"Total data symbols:       {len(data_syms)}")
    print(f"Unreferenced data:        {len(unused_data)}")

    return unused_procs, unused_data


def report_track(calls, roots, target, procs):
    if target not in procs:
        print(f"\nERROR: '{target}' is not a known proc in Main.asm")
        return

    reachable = compute_reachable(calls, roots)
    if target not in reachable:
        print(f"\n'{target}' is NOT reachable from any external root.")
        return

    path = find_shortest_path_to_target(calls, roots, target)
    print(f"\n=== TRACK: {target} ===")
    if path is None:
        print("No path found (unexpected: target was marked reachable).")
        return

    print(f"Shortest path ({len(path)} nodes):")
    print("  " + " -> ".join(path))


def main():
    parser = argparse.ArgumentParser(description="Find and optionally delete unused symbols in Main.asm")
    parser.add_argument("asm_file", nargs="?", default=ASM_PATH, help="Path to Main.asm")
    parser.add_argument("--cleanup", choices=["functions", "data", "all"], default=None,
                        help="Delete unused symbols: functions, data, or all")
    parser.add_argument("--dry-run", action="store_true",
                        help="Preview what would be deleted without writing")
    parser.add_argument("--track", metavar="PROC",
                        help="Show reachability paths from external roots to PROC")
    parser.add_argument("--roots", metavar="SUBSTRING", default='',
                        help="Show reachability paths from external roots that contain given substring")
    args = parser.parse_args()

    path = os.path.abspath(args.asm_file)
    print(f"Reading {path} ...")
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        lines = [l.split(';')[0] for l in f.readlines()]
    print(f"  {len(lines)} lines loaded.")

    procs, data_syms = collect_symbols(lines)
    print(f"  {len(procs)} procs, {len(data_syms)} data symbols found.")

    print("  Scanning for references ...")
    ref_counts = scan_references(lines, procs, data_syms, path)

    print("  Building call graph ...")
    calls, external_refs = build_call_graph(lines, procs, path)
    external_refs = {r for r in external_refs if args.roots in r}
    reachable = compute_reachable(calls, external_refs)

    print("  Computing function sizes ...")
    endps = map_proc_endps(lines, procs)
    chunks = collect_chunks(lines)
    reachable_sizes = compute_reachable_sizes(procs, endps, chunks, reachable)

    unused_procs, unused_data = report(procs, data_syms, ref_counts, reachable, reachable_sizes, external_refs)

    if args.track:
        report_track(calls, external_refs, args.track, procs)
        return

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
