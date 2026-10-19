#!/usr/bin/env python3
"""Rename a function/method and fix all compiler-reported use sites.

Usage:
  python tool/rename_function.py <old_name> <new_name>
      Textually renames (word-boundary) across all src/*.h|*.cpp|*.asm
      (visual.* always excluded), then builds to verify.

  python tool/rename_function.py <old_name> <new_name> --fixonly
      Skips the initial replacement and only runs the build/fix loop.

Call sites in visual.cpp are fixed like anywhere else; only declarations in
visual.h are never touched.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def build():
    r = subprocess.run(
        ["powershell", "-ExecutionPolicy", "Bypass", "-File", "tool/build.ps1"],
        cwd=ROOT, capture_output=True, text=True,
        encoding="utf-8", errors="replace", timeout=2400,
    )
    return r.stdout + r.stderr


def replace_on_line(path: Path, lineno: int, old: str, new: str) -> int:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines(keepends=True)
    line = lines[lineno - 1]
    new_line, n = re.subn(r"(?<![\w])" + re.escape(old) + r"(?![\w])", new, line)
    if n:
        lines[lineno - 1] = new_line
        path.write_text("".join(lines), encoding="utf-8")
    return n


def replace_everywhere(old: str, new: str) -> int:
    total = 0
    for pat in ("*.h", "*.cpp", "*.asm"):
        for f in sorted((ROOT / 'src').glob(pat)):
            if f.name.startswith("visual."):
                continue
            text = f.read_text(encoding="utf-8", errors="replace")
            new_text, n = re.subn(r"(?<![\w])" + re.escape(old) + r"(?![\w])", new, text)
            if n:
                f.write_text(new_text, encoding="utf-8")
                total += n
                print(f"  {f.relative_to(ROOT)}: {n}")
    return total


def error_sites(output: str, old: str):
    """Return {(file, line)} for errors/warnings that reference the old identifier."""
    sites = set()
    for m in re.finditer(r"^(\S+\.(?:cpp|h|asm))\((\d+),?\d*\): (?:error|warning) C\d+: .*", output, re.M):
        f, ln, msg = m.group(1), int(m.group(2)), m.group(0)
        if re.search(r"(?<![\w])" + re.escape(old) + r"(?![\w])", msg):
            sites.add((ROOT / f, ln))
    return sites


def main():
    args = sys.argv[1:]
    fixonly = "--fixonly" in args
    if fixonly:
        args.remove("--fixonly")
    lines_args = []
    if "--lines" in args:
        i = args.index("--lines")
        lines_args = args[i + 1:]
        args = args[:i]
    if len(args) != 2:
        sys.exit(__doc__)
    old, new = args

    if fixonly:
        pass
    elif lines_args:
        total = 0
        for spec in lines_args:
            fname, _, lnspec = spec.partition(":")
            path = ROOT / fname
            for ln in [int(x) for x in lnspec.split(",")]:
                n = replace_on_line(path, ln, old, new)
                total += n
                print(f"  {fname}:{ln} ({n} replaced)" if n else f"  {fname}:{ln} NOTHING REPLACED")
        if total == 0:
            sys.exit(f"ERROR: {old} not found on given lines")
    else:
        total = replace_everywhere(old, new)
        if total == 0:
            sys.exit(f"ERROR: {old} not found anywhere")

    for round_no in range(1, 26):
        out = build()
        if not re.search(r"error C\d+|error LNK\d+|: fatal error", out):
            print(f"round {round_no}: build clean")
            return
        sites = error_sites(out, old)
        if not sites:
            print(out[-4000:])
            sys.exit("ERROR: build failed with errors not mentioning the renamed function")
        print(f"round {round_no}: {len(sites)} site(s)")
        for f, ln in sorted(sites):
            if f.name == "visual.h":
                # an error in visual.h can only be a call in an inline body -
                # a mere declaration never errors. Replace it like any call site.
                print(f"  NOTE: call site inside visual.h at line {ln}")
            cnt = replace_on_line(f, ln, old, new)
            rel = f.relative_to(ROOT)
            print(f"  {rel}:{ln} ({cnt} replaced)" if cnt else f"  {rel}:{ln} NOTHING REPLACED")
    sys.exit("ERROR: too many rounds, aborting")


if __name__ == "__main__":
    main()
