r"""
Check which symbols declared in mfc_export.inc are no longer referenced in Main.asm code.

Symbols that only appear inside ASM comments (e.g. IDA's ; CODE XREF: ... annotations)
are NOT counted as referenced.

Usage:
    .\check_unused_exports.py
    .\check_unused_exports.py [--delete] [--inc <path>] [--asm <path>]
"""
import sys
import os
import re
import argparse


DEFAULT_INC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "mfc_export.inc")
DEFAULT_ASM = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "Main.asm")

# Matches:  extern SYMBOL:type
#           externdef SYMBOL :type
#           public SYMBOL
DECL_RE = re.compile(r"^\s*(extern(?:def)?|public)\s+([^\s:;,]+)", re.IGNORECASE)


def parse_symbols(inc_path):
    """Return list of (line_no, keyword, symbol, raw_line) from mfc_export.inc."""
    entries = []
    with open(inc_path, "r", encoding="utf-8", errors="replace") as f:
        for lineno, line in enumerate(f, 1):
            m = DECL_RE.match(line)
            if m:
                keyword = m.group(1).lower()
                symbol = m.group(2).strip()
                if symbol:
                    entries.append((lineno, keyword, symbol, line.rstrip()))
    return entries


def build_code_text(asm_path):
    """
    Stream Main.asm and build a single string containing only the code portions
    of each line (everything before the first semicolon).  Comments are dropped
    so that IDA's ; CODE XREF: annotations don't produce false positives.
    """
    parts = []
    with open(asm_path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            semi = line.find(";")
            if semi == -1:
                parts.append(line)
            elif semi > 0:
                parts.append(line[:semi])
            # semi == 0 → entire line is a comment, skip
    return "".join(parts)


def delete_lines(inc_path, line_numbers):
    """Remove the given 1-based line numbers from inc_path (processed in reverse order)."""
    with open(inc_path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    to_delete = sorted(set(line_numbers), reverse=True)
    for lineno in to_delete:
        del lines[lineno - 1]  # lineno is 1-based
    with open(inc_path, "w", encoding="utf-8") as f:
        f.writelines(lines)
    return len(to_delete)


def main():
    parser = argparse.ArgumentParser(description="Check unused exports in mfc_export.inc")
    parser.add_argument("--inc", default=DEFAULT_INC, help="Path to mfc_export.inc")
    parser.add_argument("--asm", default=DEFAULT_ASM, help="Path to Main.asm")
    parser.add_argument("--delete", action="store_true",
                        help="Remove unreferenced declaration lines from mfc_export.inc")
    args = parser.parse_args()

    inc_path = os.path.abspath(args.inc)
    asm_path = os.path.abspath(args.asm)

    for path, label in ((inc_path, "INC"), (asm_path, "ASM")):
        if not os.path.isfile(path):
            print(f"ERROR: {label} file not found: {path}")
            sys.exit(1)

    print(f"Parsing {inc_path} ...")
    entries = parse_symbols(inc_path)
    print(f"  Found {len(entries)} declared symbols.")

    print(f"Loading code text from {asm_path} ...")
    code_text = build_code_text(asm_path)
    print(f"  Code text size: {len(code_text) // 1024} KB")

    # Check each symbol
    unreferenced_externdef = []
    unreferenced_public = []
    referenced_count = 0

    for lineno, keyword, symbol, raw_line in entries:
        if symbol in code_text:
            referenced_count += 1
        else:
            if keyword == "public":
                unreferenced_public.append((lineno, keyword, symbol, raw_line))
            else:
                unreferenced_externdef.append((lineno, keyword, symbol, raw_line))

    total_unreferenced = len(unreferenced_externdef) + len(unreferenced_public)

    # -----------------------------------------------------------------------
    print()
    print("=" * 72)
    print("UNREFERENCED extern / externdef symbols")
    print("  (C++ symbols that ASM used to call -- no longer called from ASM)")
    print("=" * 72)
    if unreferenced_externdef:
        for lineno, keyword, symbol, raw_line in unreferenced_externdef:
            print(f"  Line {lineno:5d}: {raw_line}")
    else:
        print("  (none)")

    print()
    print("=" * 72)
    print("UNREFERENCED public symbols")
    print("  (ASM labels exported to C++ -- proc may have been migrated/deleted)")
    print("=" * 72)
    if unreferenced_public:
        for lineno, keyword, symbol, raw_line in unreferenced_public:
            print(f"  Line {lineno:5d}: {raw_line}")
    else:
        print("  (none)")

    print()
    print(f"Summary: {total_unreferenced} unreferenced out of {len(entries)} declared "
          f"({referenced_count} still referenced in code)")
    print()

    if not args.delete:
        print("NOTE: Results are candidates only. Verify manually before removing --")
        print("      data symbols accessed indirectly won't appear by name in ASM.")
    else:
        if total_unreferenced == 0:
            print("\nNothing to delete.")
        else:
            all_unreferenced = unreferenced_externdef + unreferenced_public
            line_numbers = [lineno for lineno, _, _, _ in all_unreferenced]
            deleted = delete_lines(inc_path, line_numbers)
            print(f"\nDeleted {deleted} lines from {inc_path}")


if __name__ == "__main__":
    main()
