#!/usr/bin/env python3
"""Batch driver: run tool/rename_field.py + commit_rename.sh for a list of renames.

Usage: python tool/rename_field_batch.py <items-file>
Each line of items-file: header|line|old|new|commit-msg|rename.txt-line
Aborts on the first failure (build not clean or commit gate failed).
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def main() -> None:
    items = []
    for raw in Path(sys.argv[1]).read_text(encoding="utf-8").splitlines():
        raw = raw.rstrip("\n")
        if not raw or raw.startswith("#"):
            continue
        parts = raw.split("|")
        if len(parts) != 6:
            sys.exit(f"bad line (need 6 |-separated fields): {raw!r}")
        items.append(parts)

    for hdr, ln, old, new, msg, line in items:
        r = subprocess.run(
            [sys.executable, "tool/rename_field.py", f"{hdr}:{ln}", old, new],
            cwd=ROOT, capture_output=True, text=True,
        )
        out = r.stdout + r.stderr
        if r.returncode != 0 or "build clean" not in out:
            print(f"FAILED {msg}\n{out[-1500:]}")
            sys.exit(1)
        print(f"renamed {old}@{ln} -> {new}")
        c = subprocess.run(
            ["bash", "tool/commit_rename.sh", msg, line],
            cwd=ROOT, capture_output=True, text=True,
        )
        if c.returncode != 0 or "OK:" not in c.stdout:
            print(f"COMMIT FAILED {msg}\n{c.stdout[-1500:]}{c.stderr[-500:]}")
            sys.exit(1)
        print(f"committed: {msg}")
    print(f"ALL DONE ({len(items)} renames)")


if __name__ == "__main__":
    main()
