#!/usr/bin/env python3
"""Rename a class field and fix all compiler-reported use sites.

Usage: python tool/rename_field.py <file:lines> <old_name> <new_name>
       e.g. python tool/rename_field.py src/packet.h:28 field_0x4 order_mode

1. Renames <old_name> (word-boundary) on the given line(s) of the file.
2. Builds, parses compiler errors naming <old_name>, replaces the identifier
   on exactly those reported lines, rebuilds, repeats until clean.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def replace_on_line(path: Path, lineno: int, old: str, new: str) -> int:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines(keepends=True)
    line = lines[lineno - 1]
    new_line, n = re.subn(r"(?<![\w])" + re.escape(old) + r"(?![\w])", new, line)
    if n:
        lines[lineno - 1] = new_line
        path.write_text("".join(lines), encoding="utf-8")
    return n


def build():
    r = subprocess.run(
        ["powershell", "-ExecutionPolicy", "Bypass", "-File", "tool/build.ps1"],
        cwd=ROOT, capture_output=True, text=True,
        encoding="utf-8", errors="replace", timeout=1200,
    )
    return r.stdout + r.stderr


def error_sites(output: str, old: str):
    """Return {(file, line)} for errors that reference the old identifier."""
    sites = set()
    for m in re.finditer(r"^(\S+\.(?:cpp|h))\((\d+),\d+\): (?:error|warning) C\d+: .*", output, re.M):
        f, ln, msg = m.group(1), int(m.group(2)), m.group(0)
        if re.search(r"(?<![\w])" + re.escape(old) + r"(?![\w])", msg):
            sites.add((ROOT / f, ln))
    return sites


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    loc, old, new = sys.argv[1:]
    fname, _, lnspec = loc.partition(":")
    path = ROOT / fname
    if not lnspec:
        sys.exit("ERROR: pass line number(s), e.g. src/packet.h:28")
    lines = [int(x) for x in lnspec.split(",")]

    total = 0
    for ln in lines:
        total += replace_on_line(path, ln, old, new)
    if total == 0:
        print(f"note: {old} not found on {loc} (maybe already renamed); fixing use sites only")

    for round_no in range(1, 26):
        out = build()
        if "error" not in out and "LNK" not in out:
            print(f"round {round_no}: build clean")
            return
        sites = error_sites(out, old)
        if not sites:
            print(out[-4000:])
            sys.exit("ERROR: build failed with errors not mentioning the renamed field")
        print(f"round {round_no}: {len(sites)} site(s)")
        for f, ln in sorted(sites):
            cnt = replace_on_line(f, ln, old, new)
            rel = f.relative_to(ROOT)
            print(f"  {rel}:{ln} ({cnt} replaced)" if cnt else f"  {rel}:{ln} NOTHING REPLACED")
    sys.exit("ERROR: too many rounds, aborting")


if __name__ == "__main__":
    main()
