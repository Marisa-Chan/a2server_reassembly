#!/usr/bin/env python
"""Vis* recheck loop: one-function-at-a-time fidelity audit of every migrated Vis*
function against the original binary. Protocol: AGENTS.md "Vis* Recheck Loop".

State is a single append-only log, migration_state/vis_recheck_log.txt. There is
deliberately NO queue file: `next` rescans src/ live and subtracts logged addresses,
so the state can never go stale against HEAD. Old verify_queue.txt statuses are
irrelevant to this sweep - every Vis* function starts PENDING here.

Scope: classes named Vis* (VisScreen, VisShop, ...), except `Visibility` (server-side
logic that only shares the prefix). ~996 functions over 93 classes.

    python tool/vis_recheck.py next                    # next PENDING function
    python tool/vis_recheck.py status                  # progress per class
    python tool/vis_recheck.py list [--class VisShop]  # all PENDING items
    python tool/vis_recheck.py ok <addr> --note "trickiest point verified"
    python tool/vis_recheck.py ok <addr> --fixed --note "what was wrong + the fix"
    python tool/vis_recheck.py skip <addr> --note "why it cannot be compared"

`ok`/`skip` append the log line, stage only the log, and fold it into history:
  - HEAD is neither a `Fix ...` commit nor pushed to any remote -> git commit --amend
    (this is the "amend the line into the last non-fix commit" rule);
  - otherwise -> standalone "vis-recheck: verification log" carrier commit, which the
    next ok then amends into. Pushed history and fix commits are never rewritten.
Fix commits are made by hand: one function per commit, `tool/build.ps1` green first,
message starting with `Fix` (the fold logic keys on that prefix). --no-fold leaves the
log line uncommitted instead.
"""
import argparse
import datetime
import glob
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import verify_queue as vq  # reuse its // ADDR comment scanner

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG = os.path.join(ROOT, "migration_state", "vis_recheck_log.txt")
LOG_REL = "migration_state/vis_recheck_log.txt"

# Vis-prefixed classes that are NOT part of the visual UI family.
EXCLUDED_CLASSES = {"Visibility"}

LOG_LINE = re.compile(r"^(?P<status>OK|FIX|SKIP)\t(?P<addr>[0-9A-Fa-f]{6})\t(?P<name>\S+)\t(?P<date>\S+)\t(?P<note>.*)$")

LOG_HEADER = """\
# Vis* recheck log - per-function fidelity audit against the original binary.
# Protocol: AGENTS.md "Vis* Recheck Loop". Append-only; done-detection keys on the address.
# Lines: STATUS<TAB>ADDR<TAB>Class::method<TAB>YYYY-MM-DD<TAB>note
# STATUS: OK = verified line-by-line vs the binary (note = trickiest point checked)
#         FIX = bug found and fixed during this sweep (note = what was wrong)
#         SKIP = could not be compared (note = reason); review these separately
"""


def scan():
    """All Vis* migrated definitions: [(class, addr_int, ADDR, qname, file)], sorted."""
    entries = []
    files = sorted(glob.glob(os.path.join(vq.SRC, "*.cpp")) + glob.glob(os.path.join(vq.SRC, "*.h")))
    for path in files:
        found, _ = vq.find_definitions(path)
        rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
        for addr, qname, _lineno in found:
            cls = qname.rsplit("::", 1)[0] if "::" in qname else ""
            if cls.startswith("Vis") and cls not in EXCLUDED_CLASSES:
                entries.append((cls, int(addr, 16), addr.upper(), qname, rel))
    entries.sort()
    return entries


def read_log():
    """addr -> (status, qname, note); last line for an address wins."""
    done = {}
    if os.path.isfile(LOG):
        with open(LOG, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                m = LOG_LINE.match(line.rstrip("\n"))
                if m:
                    done[m.group("addr").upper()] = (m.group("status"), m.group("name"), m.group("note"))
    return done


def pending(entries, done):
    return [e for e in entries if e[2] not in done]


def git(*args):
    return subprocess.run(["git", *args], cwd=ROOT, check=True, capture_output=True, text=True).stdout


def fold_log(no_fold):
    """Stage only the log and fold the appended line into history (see module docstring)."""
    if no_fold:
        print(f"log line left uncommitted in {LOG_REL}")
        return
    staged = git("diff", "--cached", "--name-only").split()
    if set(staged) - {LOG_REL.replace("/", os.sep), LOG_REL}:
        print("NOT folding: other files are staged (%s); log line left uncommitted" % ", ".join(staged))
        return
    git_dir = git("rev-parse", "--git-dir").strip()
    if any(os.path.exists(os.path.join(git_dir, d)) for d in ("MERGE_HEAD", "rebase-merge", "rebase-apply")):
        print("NOT folding: rebase/merge in progress; log line left uncommitted")
        return
    subject = git("log", "-1", "--format=%s").strip()
    pushed = git("branch", "-r", "--contains", "HEAD").strip()
    if subject.startswith("Fix"):
        git("commit", "-m", "vis-recheck: verification log")
        print("HEAD is a fix commit; log folded into a new carrier commit")
    elif pushed:
        git("commit", "-m", "vis-recheck: verification log")
        print("HEAD is already pushed (%s); log folded into a new carrier commit" % pushed.split()[0])
    else:
        git("commit", "--amend", "--no-edit")
        print('log amended into HEAD: "%s"' % subject)


def append_log(status, addr, qname, note, force, no_fold):
    done = read_log()
    key = addr.upper()
    if key in done and not force:
        sys.exit("error: %s already logged as %s (%s); use --force to append a re-check" % (key, done[key][0], done[key][1]))
    os.makedirs(os.path.dirname(LOG), exist_ok=True)
    header_needed = not os.path.isfile(LOG)
    with open(LOG, "a", encoding="utf-8", newline="\n") as fh:
        if header_needed:
            fh.write(LOG_HEADER)
        fh.write("%s\t%s\t%s\t%s\t%s\n" % (status, key, qname, datetime.date.today().isoformat(), note.replace("\t", " ").strip()))
    git("add", LOG_REL)
    fold_log(no_fold)


def cmd_next(_args):
    entries = scan()
    done = read_log()
    todo = pending(entries, done)
    if not todo:
        print("DONE: all %d Vis* functions checked (%d OK, %d FIX, %d SKIP)" % (
            len(entries),
            sum(1 for s, _, _ in done.values() if s == "OK"),
            sum(1 for s, _, _ in done.values() if s == "FIX"),
            sum(1 for s, _, _ in done.values() if s == "SKIP")))
        return
    cls, _a, addr, qname, rel = todo[0]
    print("NEXT  %s  %s  (%s, %s)  [%d/%d done]" % (addr, qname, rel, cls, len(entries) - len(todo), len(entries)))
    print("  python tool/gh.py decompile %s" % addr)
    print("  python tool/gh.py disasm %s" % addr)
    print("  -> fix + build + `Fix %s: ...` commit, then: ok %s --fixed --note \"...\"" % (qname, addr))
    print("  -> or if clean: ok %s --note \"trickiest point verified\"" % addr)


def cmd_status(_args):
    entries = scan()
    done = read_log()
    todo = pending(entries, done)
    by_class = {}
    for cls, _a, addr, qname, rel in todo:
        by_class.setdefault(cls, []).append(qname)
    print("Vis* recheck: %d/%d done, %d pending" % (len(entries) - len(todo), len(entries), len(todo)))
    counts = {"OK": 0, "FIX": 0, "SKIP": 0}
    for s, _, _ in done.values():
        if s in counts:
            counts[s] += 1
    print("logged: %d OK, %d FIX, %d SKIP" % (counts["OK"], counts["FIX"], counts["SKIP"]))
    if todo:
        print("next: %s %s (%s)" % (todo[0][2], todo[0][3], todo[0][0]))
    if counts["SKIP"]:
        print("SKIPped items to review:")
        for addr, (s, name, note) in sorted(done.items()):
            if s == "SKIP":
                print("  %s  %s  (%s)" % (addr, name, note))


def cmd_list(args):
    entries = scan()
    done = read_log()
    todo = pending(entries, done)
    shown = 0
    for cls, _a, addr, qname, rel in todo:
        if args.cls and cls != args.cls:
            continue
        print("%s\t%s\t%s" % (addr, qname, rel))
        shown += 1
    if not shown:
        print("no pending items%s" % (" for class %s" % args.cls if args.cls else ""))


def cmd_ok(args):
    entries = {e[2]: e for e in scan()}
    key = args.addr.upper()
    if key not in entries:
        print("warning: %s is not in the current Vis* scan (renamed or removed?); logging anyway" % key)
    qname = entries[key][3] if key in entries else args.addr
    status = "FIX" if args.fixed else "OK"
    append_log(status, key, qname, args.note, args.force, args.no_fold)


def cmd_skip(args):
    entries = {e[2]: e for e in scan()}
    key = args.addr.upper()
    qname = entries[key][3] if key in entries else args.addr
    append_log("SKIP", key, qname, args.note, args.force, args.no_fold)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    sub.add_parser("next", help="next PENDING function").set_defaults(func=cmd_next)
    sub.add_parser("status", help="progress summary").set_defaults(func=cmd_status)

    p_list = sub.add_parser("list", help="all PENDING items")
    p_list.add_argument("--class", dest="cls", help="restrict to one class, e.g. VisShop")
    p_list.set_defaults(func=cmd_list)

    p_ok = sub.add_parser("ok", help="record a checked function (clean or fixed)")
    p_ok.add_argument("addr", help="original function address, e.g. 4BCB63")
    p_ok.add_argument("--note", required=True, help="trickiest point verified / what was wrong")
    p_ok.add_argument("--fixed", action="store_true", help="a bug was found and fixed (logs FIX)")
    p_ok.add_argument("--force", action="store_true", help="append a re-check for an already-logged address")
    p_ok.add_argument("--no-fold", action="store_true", help="leave the log line uncommitted")
    p_ok.set_defaults(func=cmd_ok)

    p_skip = sub.add_parser("skip", help="record a function that cannot be compared")
    p_skip.add_argument("addr")
    p_skip.add_argument("--note", required=True, help="why it cannot be compared")
    p_skip.add_argument("--force", action="store_true")
    p_skip.add_argument("--no-fold", action="store_true")
    p_skip.set_defaults(func=cmd_skip)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
