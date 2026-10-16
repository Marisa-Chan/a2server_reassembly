#!/usr/bin/env python
"""Generate migration_state/verify_queue.txt: migrated C++ functions to re-check against Ghidra.

Every migrated function carries its original address in a `// ADDR` comment, in one of two
styles (both are used across the sources):

    // 55ECFE                      Effect::Effect()
    void QuestMap::sub_55ECFE()    {
    {                                  //53ec02
        ...                            ...
    }                                }

This script collects all of them from src/*.cpp and src/*.h and writes a queue grouped by
class (free functions grouped by file):

    STATUS<TAB>ADDR<TAB>Class::method<TAB>src/file.cpp[<TAB>note]

Locations carry no line number on purpose: fixes shift lines. Find a function by grepping
its `// ADDR` comment in the file.

Statuses: PENDING (new entries), OK (checked, no fix needed - matches the decompile, or the
difference is a decompiler artifact / intentional correction), FIXED (was wrong, fixed),
SUSPECT (substantial disagreement, needs arbitration), SKIP (could not compare, e.g. degraded
decompile).
Existing statuses and notes survive regeneration (keyed by address); locations and
grouping are refreshed from the source.

    python tool/verify_queue.py             # regenerate the queue, print a summary
    python tool/verify_queue.py --dry-run   # summary only, do not write the file
"""
import argparse
import glob
import os
import re
from collections import Counter, defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
QUEUE = os.path.join(ROOT, "migration_state", "verify_queue.txt")

ADDR_LINE = re.compile(r"^\s*//\s*([0-9A-Fa-f]{6})\s*$")
NAME_RE = re.compile(r"(?:(?P<cls>[A-Za-z_]\w*)::)?(?P<name>operator\S*|~?[A-Za-z_]\w*)\s*\(")
QUEUE_LINE = re.compile(r"^(?P<status>[A-Z]+)\t(?P<addr>[0-9A-Fa-f]{6})\t(?P<name>\S+)\t(?P<loc>\S+)(?:\t(?P<note>.*))?$")
SKIP_WORDS = {"if", "for", "while", "switch", "catch", "return", "sizeof", "ASSERT_OFFSET", "ASSERT_SIZE"}
STATUSES = ("PENDING", "OK", "FIXED", "SUSPECT", "SKIP")


def strip_comment(line):
    return line.split("//")[0]


def qname_of(m):
    return f"{m.group('cls')}::{m.group('name')}" if m.group("cls") else m.group("name")


def has_initializer_eq(sig):
    """True if a `=` sits before the first `(` (data initializer, not a function).
    The `=` of an `operator=`/`operator==` name does not count."""
    prefix = sig.split("(", 1)[0]
    prefix = re.sub(r"operator\S*\s*$", "", prefix)
    return "=" in prefix


def match_forward(lines, i):
    """`// ADDR` directly above the definition. Return (qname, lineno) or None."""
    j = i + 1
    if j < len(lines) and strip_comment(lines[j]).strip().startswith("template"):
        j += 1
    if j >= len(lines):
        return None
    head = strip_comment(lines[j])
    # the name's `(` must sit on the first signature line; a `=` before it means this is
    # a data initializer, not a function (`int x[5] = { Foo(1) }`)
    sig = head
    if "(" not in head and not any(c in head for c in "=;{") and j + 1 < len(lines):
        sig = head + " " + strip_comment(lines[j + 1])
    if "(" not in sig or has_initializer_eq(sig) or not is_definition(lines, j):
        return None
    m = NAME_RE.search(sig)
    if not m or m.group("name") in SKIP_WORDS:
        return None
    return qname_of(m), j + 1


def match_backward(lines, i):
    """`// ADDR` as the first (non-empty) line of a function body. Return (qname, lineno) or None."""
    b = i - 1
    while b >= 0 and not lines[b].strip():
        b -= 1
    if b < 0:
        return None
    prev = strip_comment(lines[b]).rstrip()
    if not prev.endswith("{"):
        return None
    parts = [prev[:-1].rstrip()]  # signature tail without the body brace
    start = b
    while True:
        sig = " ".join(p for p in parts if p)
        if "(" in sig:
            break
        k = start - 1
        if k < 0 or k < b - 5:
            break
        line = strip_comment(lines[k]).strip()
        if not line or line[-1] in ";{}:" or line.startswith("#"):
            break
        parts.insert(0, line)
        start = k
    sig = " ".join(p for p in parts if p)
    if "(" not in sig or "}" in sig or has_initializer_eq(sig):
        return None
    m = NAME_RE.search(sig)
    if not m or m.group("name") in SKIP_WORDS:
        return None
    return qname_of(m), start + 1


def is_definition(lines, j):
    """True if a definition follows: a `{` body at paren depth 0, or `= default;` / `= delete;`."""
    depth = 0
    params_done = False
    tail = ""
    for k in range(j, min(j + 10, len(lines))):
        for ch in strip_comment(lines[k]):
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    params_done = True
            elif depth == 0:
                if ch == "{":
                    return True
                if ch == ";":
                    return params_done and re.fullmatch(r"\s*=\s*(default|delete)\s*", tail) is not None
                if params_done:
                    tail += ch
    return False


def find_definitions(path):
    """Return (found, skipped) for one file. found = [(addr, qname, lineno)]; skipped =
    [(addr, lineno)] for address comments tied to no definition (e.g. header declarations)."""
    lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
    found, skipped = [], []
    for i, raw in enumerate(lines):
        m = ADDR_LINE.match(raw)
        if not m:
            continue
        hit = match_forward(lines, i) or match_backward(lines, i)
        if hit:
            qname, lineno = hit
            found.append((m.group(1).upper(), qname, lineno))
        else:
            skipped.append((m.group(1).upper(), i + 1))
    return found, skipped


def load_queue(path):
    """addr -> (status, note) from the existing queue file."""
    old = {}
    if os.path.isfile(path):
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                m = QUEUE_LINE.match(line.rstrip("\n"))
                if m:
                    old[m.group("addr").upper()] = (m.group("status"), m.group("note") or "")
    return old


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    old = load_queue(QUEUE)

    entries = {}  # addr -> dict(name, loc, group)
    dups = defaultdict(list)  # addr -> [(qname, loc)] of extra definitions sharing the address
    skipped = []
    files = sorted(glob.glob(os.path.join(SRC, "*.cpp")) + glob.glob(os.path.join(SRC, "*.h")))
    for path in files:
        rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
        found, sk = find_definitions(path)
        skipped.extend((rel, addr, ln) for addr, ln in sk)
        for addr, qname, lineno in found:
            if addr in entries:
                dups[addr].append((qname, f"{rel}:{lineno}"))
                continue
            group = qname.rsplit("::", 1)[0] if "::" in qname else os.path.splitext(os.path.basename(rel))[0] + " (globals)"
            entries[addr] = {"name": qname, "loc": rel, "group": group}

    groups = defaultdict(list)
    for addr, e in entries.items():
        status, note = old.get(addr, ("PENDING", ""))
        groups[e["group"]].append((status, addr, e["name"], e["loc"], note))

    out = [
        "# Ghidra verification queue - migrated C++ functions awaiting re-check against the decompiler.",
        "# Auto-generated by tool/verify_queue.py; statuses and notes are preserved across regeneration.",
        "# Columns: STATUS<TAB>ADDR<TAB>Class::method<TAB>src/file.cpp[<TAB>note]",
        "# Locations carry no line number on purpose (fixes shift lines); grep the // ADDR comment in the file.",
        "# Statuses: PENDING | OK | FIXED | SUSPECT | SKIP",
        "# Protocol: see AGENTS.md, \"Verifying Migrated Functions Against Ghidra\".",
        "",
    ]
    counts = Counter()
    for group in sorted(groups, key=str.lower):
        rows = sorted(groups[group], key=lambda r: int(r[1], 16))
        out.append(f"## {group} ({len(rows)})")
        for status, addr, name, loc, note in rows:
            counts[status] += 1
            line = f"{status}\t{addr}\t{name}\t{loc}"
            if note:
                line += f"\t{note}"
            out.append(line)
        out.append("")

    new = sorted(set(entries) - set(old))
    dropped = sorted(set(old) - set(entries))
    print(f"Scanned {len(files)} files: {len(entries)} migrated definitions "
          f"({sum(len(v) for v in dups.values())} duplicate addresses, {len(skipped)} address comments without a definition skipped)")
    print("Statuses: " + ", ".join(f"{s}={counts.get(s, 0)}" for s in STATUSES))
    if new:
        print(f"New since last generation: {len(new)}")
    if dropped:
        print(f"Dropped (address no longer in source): {len(dropped)}")
        for addr in dropped[:15]:
            print(f"  {addr}  {old[addr][0]}")
    if dups:
        print("Duplicate addresses (first definition kept in the queue):")
        for addr in sorted(dups):
            extra = ", ".join(f"{n} at {l}" for n, l in dups[addr])
            print(f"  {addr}  {entries[addr]['name']} at {entries[addr]['loc']}  (also: {extra})")
    if skipped:
        print("Address comments without a following definition (header declarations are expected here):")
        for rel, addr, ln in skipped[:15]:
            print(f"  {rel}:{ln}  {addr}")
        if len(skipped) > 15:
            print(f"  ... and {len(skipped) - 15} more")
    print("Pending by group:")
    for group in sorted(groups, key=str.lower):
        pending = sum(1 for r in groups[group] if r[0] == "PENDING")
        if pending:
            print(f"  {group}: {pending}")

    if not args.dry_run:
        os.makedirs(os.path.dirname(QUEUE), exist_ok=True)
        with open(QUEUE, "w", encoding="utf-8") as fh:
            fh.write("\n".join(out))
        print(f"Wrote {os.path.relpath(QUEUE, ROOT)}")


if __name__ == "__main__":
    main()
