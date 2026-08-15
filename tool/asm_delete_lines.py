"""
Delete line ranges from Main.asm (bottom-up to preserve line numbers).

Usage:
    python tool/asm_delete_lines.py <start-end> [<start-end> ...]
    python tool/asm_delete_lines.py --dry-run <start-end> [<start-end> ...]
    python tool/asm_delete_lines.py --asm-file path/to/Main.asm <start-end> [...]
"""
import sys
import os
import argparse

sys.stdout.reconfigure(encoding='utf-8', errors='replace')


def main():
    parser = argparse.ArgumentParser(description="Delete line ranges from Main.asm")
    parser.add_argument("ranges", nargs="+", help="Line ranges as start-end (1-based, inclusive)")
    parser.add_argument("--dry-run", action="store_true", help="Show what would be deleted without modifying")
    parser.add_argument("--asm-file", default=None, help="Path to Main.asm")
    args = parser.parse_args()

    if args.asm_file:
        asm_file = args.asm_file
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        asm_file = os.path.join(script_dir, "..", "src", "Main.asm")

    asm_file = os.path.abspath(asm_file)
    if not os.path.isfile(asm_file):
        print(f"ERROR: File not found: {asm_file}")
        sys.exit(1)

    # Parse ranges
    parsed = []
    for r in args.ranges:
        parts = r.split("-")
        if len(parts) != 2:
            print(f"ERROR: Invalid range format '{r}'. Use 'start-end' (e.g. 100-200).")
            sys.exit(1)
        try:
            s, e = int(parts[0]), int(parts[1])
        except ValueError:
            print(f"ERROR: Invalid range format '{r}'. Use 'start-end' (e.g. 100-200).")
            sys.exit(1)
        if s > e:
            print(f"ERROR: Invalid range {r} (start > end)")
            sys.exit(1)
        parsed.append((s, e))

    # Sort descending by start line (delete from bottom up)
    parsed.sort(key=lambda x: x[0], reverse=True)

    # Check for overlaps
    for i in range(len(parsed) - 1):
        if parsed[i][0] <= parsed[i + 1][1]:
            print(f"ERROR: Overlapping ranges: {parsed[i+1][0]}-{parsed[i+1][1]} and {parsed[i][0]}-{parsed[i][1]}")
            sys.exit(1)

    with open(asm_file, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    original_count = len(lines)
    total_deleted = 0

    print(f"File: {asm_file} ({original_count} lines)")
    print()

    for s, e in parsed:
        count = e - s + 1
        si = s - 1  # 0-based
        ei = e - 1

        if si < 0 or ei >= len(lines):
            print(f"ERROR: Range {s}-{e} is out of bounds (file has {len(lines)} lines)")
            sys.exit(1)

        first_line = lines[si].rstrip()
        last_line = lines[ei].rstrip()
        if len(first_line) > 80:
            first_line = first_line[:77] + "..."
        if len(last_line) > 80:
            last_line = last_line[:77] + "..."

        print(f"  Delete lines {s}-{e} ({count} lines)")
        print(f"    First: {first_line}")
        print(f"    Last:  {last_line}")

        if not args.dry_run:
            del lines[si:e]  # e is already 1-based, so si:e deletes si..ei inclusive
            total_deleted += count

    if args.dry_run:
        total = sum(e - s + 1 for s, e in parsed)
        print()
        print(f"DRY RUN: Would delete {total} lines total.")
    else:
        with open(asm_file, "w", encoding="utf-8", newline="") as f:
            f.writelines(lines)
        print()
        print(f"Deleted {total_deleted} lines. File now has {len(lines)} lines (was {original_count}).")


if __name__ == "__main__":
    main()
