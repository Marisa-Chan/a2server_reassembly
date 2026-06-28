"""
Rename/replace a symbol throughout Main.asm (proc/endp labels, call sites, comments, etc.)

Usage:
    python asm_rename.py <old_name> <new_name>
    python asm_rename.py --dry-run <old_name> <new_name>
    python asm_rename.py --asm-file path/to/Main.asm <old_name> <new_name>
"""
import sys
import os
import re
import argparse

sys.stdout.reconfigure(encoding='utf-8', errors='replace')


def main():
    parser = argparse.ArgumentParser(description="Rename a symbol throughout Main.asm")
    parser.add_argument("old_name", help="Current symbol name (e.g. sub_560DC2)")
    parser.add_argument("new_name", help="New symbol name (e.g. ?Method@Class@@QAE@XZ)")
    parser.add_argument("--dry-run", action="store_true", help="Show matches without modifying")
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

    with open(asm_file, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    # Word-boundary matching to avoid partial replacements
    if '?' in args.old_name:
        pattern = re.escape(args.old_name)
    else:
        pattern = rf"\b{re.escape(args.old_name)}\b"

    matches = list(re.finditer(pattern, content))
    match_count = len(matches)

    if match_count == 0:
        print(f"No occurrences of '{args.old_name}' found.")
        sys.exit(0)

    print(f"Found {match_count} occurrences of '{args.old_name}'")

    if args.dry_run:
        lines = content.split("\n")
        shown = 0
        for i, line in enumerate(lines):
            if shown >= 20:
                break
            if re.search(pattern, line):
                line_num = i + 1
                trimmed = line.rstrip()
                if len(trimmed) > 120:
                    trimmed = trimmed[:117] + "..."
                print(f"  L{line_num}: {trimmed}")
                shown += 1
        if shown < match_count:
            print(f"  ... and {match_count - shown} more")
        print()
        print(f"DRY RUN: Would replace {match_count} occurrences.")
    else:
        new_content = re.sub(pattern, args.new_name, content)
        with open(asm_file, "w", encoding="utf-8", newline="") as f:
            f.write(new_content)
        print(f"Replaced {match_count} occurrences of '{args.old_name}' -> '{args.new_name}'")


if __name__ == "__main__":
    main()
