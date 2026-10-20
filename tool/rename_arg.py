#!/usr/bin/env python3
"""Rename a placeholder argument name on exact line numbers of a file.

Usage:
  python tool/rename_arg.py --file src/gameobj.cpp --lines 378-379 --old arg1 --new selected [--dry-run]

Whole-word replacement, only on the given 1-based lines. Checks that the old
name actually occurs on every listed line (unless --allow-missing).
"""
import argparse
import re
import sys


def parse_ranges(spec):
    lines = []
    for part in spec.split(","):
        if "-" in part:
            a, b = part.split("-")
            lines.extend(range(int(a), int(b) + 1))
        else:
            lines.append(int(part))
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--file", required=True)
    ap.add_argument("--lines", required=True, help="1-based line numbers/ranges, e.g. 12,40-44")
    ap.add_argument("--old", required=True)
    ap.add_argument("--new", required=True)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--allow-missing", action="store_true", help="don't fail if a listed line lacks the old name")
    args = ap.parse_args()

    targets = set(parse_ranges(args.lines))
    pat = re.compile(r"\b%s\b" % re.escape(args.old))

    with open(args.file, "r", encoding="utf-8", errors="replace", newline="") as f:
        text = f.read()
    lines = text.split("\n")

    missing = [n for n in sorted(targets) if not pat.search(lines[n - 1])]
    if missing and not args.allow_missing:
        print(f"ERROR: {args.old} not found on lines {missing} of {args.file}")
        for n in missing:
            print(f"  {n}: {lines[n-1].strip()[:120]}")
        sys.exit(1)

    changed = 0
    for n in sorted(targets):
        new_line, cnt = pat.subn(args.new, lines[n - 1])
        if cnt:
            changed += cnt
            lines[n - 1] = new_line
            print(f"{args.file}:{n}: {lines[n-1].strip()[:110]}")
        else:
            print(f"{args.file}:{n}: (no match)")

    if args.dry_run:
        print("dry run, not written")
        return

    with open(args.file, "w", encoding="utf-8", errors="replace", newline="") as f:
        f.write("\n".join(lines))
    print(f"{changed} occurrence(s) replaced in {args.file}")


if __name__ == "__main__":
    main()
