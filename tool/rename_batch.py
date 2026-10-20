"""Batch-run rename_function.py over old|new pairs, sequentially, reporting failures.

Usage: python tool/rename_batch.py < specs.txt   (one `old|new|class|reason` per line)

Unlike shell loops piping through `tail`, this checks rename_function.py's real
exit code before committing, so a failed rename can never be silently skipped
or committed half-done. Only use for names unique across the tree (sub_*/FUN_*);
shared VMethod* names still need explicit --lines specs per rename_function.py.
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def main():
    specs = [l.strip() for l in sys.stdin if l.strip()]
    failures = []
    for spec in specs:
        old, new, cls, why = (spec.split('|', 3) + [''])[:4]
        r = subprocess.run([sys.executable, 'tool/rename_function.py', old, new],
                           cwd=ROOT, capture_output=True, text=True, timeout=2400)
        out = (r.stdout + r.stderr).strip().splitlines()
        tail = out[-1] if out else '(no output)'
        if r.returncode != 0:
            failures.append(f"{old} -> {new}: {tail}")
            print(f"FAIL {old} -> {new}: {tail}", flush=True)
            continue
        subprocess.run(['git', 'add', '-u'], cwd=ROOT, check=True)
        subprocess.run(['git', 'commit', '-q', '-m', f"Rename {cls}::{old} to {new}"], cwd=ROOT, check=True)
        with open(ROOT / 'migration_state/rename.txt', 'a', encoding='utf-8') as f:
            f.write(f"{cls}::{old} -> {new} -- {why}\n")
        print(f"ok   {old} -> {new}", flush=True)
    print(f"DONE: {len(specs) - len(failures)} ok, {len(failures)} failed")


if __name__ == '__main__':
    main()
