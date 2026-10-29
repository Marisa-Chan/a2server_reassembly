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

Families (`--family`, or A2_RECHECK_FAMILY env var; default `vis`):
  vis        - the Vis* sweep above (log: migration_state/vis_recheck_log.txt)
  mainwindow - MainWindow fidelity audit. Same loop, but the binary ground truth is
               DUAL: server `gh.py decompile/disasm <srv>` AND, when the source carries
               a `(client: <va>)` pair, `gh.py --client decompile <cli>`. Divergences
               between the two binaries are gated in C++ with #ifdef A2SERVER/A2CLIENT.
               Log: migration_state/mainwindow_recheck_log.txt. Client-only methods
               (e.g. RunModalLoopClient) are keyed by their client address.

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
from collections import namedtuple

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import verify_queue as vq  # reuse its // ADDR comment scanner

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG = os.path.join(ROOT, "migration_state", "vis_recheck_log.txt")
LOG_REL = "migration_state/vis_recheck_log.txt"

# Vis-prefixed classes that are NOT part of the visual UI family.
EXCLUDED_CLASSES = {"Visibility"}

# Audit families: each has its own class filter, log file and ground-truth story.
FAMILIES = {
    "vis": {
        "log_rel": "migration_state/vis_recheck_log.txt",
        "label": "Vis*",
    },
    "mainwindow": {
        "log_rel": "migration_state/mainwindow_recheck_log.txt",
        "label": "MainWindow",
    },
}
FAMILY = "vis"  # set from --family / A2_RECHECK_FAMILY in main()

MAINWINDOW_CPP = os.path.join(vq.SRC, "main_window.cpp")
MAINWINDOW_H = os.path.join(vq.SRC, "main_window.h")

# `// 48cb3c` and `// 49152f  client 495c94` (whole-line comments only)
ADDR_STANDALONE = re.compile(
    r"^\s*//\s*(?P<srv>[0-9A-Fa-f]{6})(?:\s*\(?\s*client[:\s]+(?P<cli>[0-9A-Fa-f]{6}))?\s*$")
# `// client: 490ce4` - client-only definition marker or supplement under an addr comment
ADDR_CLIENT_ONLY = re.compile(r"^\s*//\s*client[:\s]+(?P<cli>[0-9A-Fa-f]{6})\b")
# `void MainWindow::Foo(...) { //484ab0` / `MainWindow::~MainWindow() { // 4961b0`
# / `int32_t MainWindow::RunModalLoopClient() { //client 4949cd`
DEF_TRAILING = re.compile(
    r"^\s*(?:[A-Za-z_][\w:<>&*\s]*\s*)?\bMainWindow::(~?[A-Za-z_]\w*)\s*\([^;{}]*\)[^;{}]*\{"
    r"\s*//\s*(?:(?P<srv>[0-9A-Fa-f]{6})\s*)?(?:\(?\s*client[:\s]+(?P<cli>[0-9A-Fa-f]{6}))?")
# `void MainWindow::Foo(...)` definition line below a standalone addr comment
DEF_PLAIN = re.compile(r"^\s*(?:[A-Za-z_][\w:<>&*\s]*\s)?MainWindow::(~?[A-Za-z_]\w*)\s*\(")
# scan() entry: ADDR is the log key (server address, or client address for client-only
# methods); srv/cli hold the raw addresses (either may be None).
Entry = namedtuple("Entry", "cls addr ADDR qname rel srv cli")

LOG_LINE = re.compile(r"^(?P<status>OK|FIX|SKIP)\t(?P<addr>[0-9A-Fa-f]{6})\t(?P<name>\S+)\t(?P<date>\S+)\t(?P<note>.*)$")

LOG_HEADER = """\
# Vis* recheck log - per-function fidelity audit against the original binary.
# Protocol: AGENTS.md "Vis* Recheck Loop". Append-only; done-detection keys on the address.
# Lines: STATUS<TAB>ADDR<TAB>Class::method<TAB>YYYY-MM-DD<TAB>note
# STATUS: OK = verified line-by-line vs the binary (note = trickiest point checked)
#         FIX = bug found and fixed during this sweep (note = what was wrong)
#         SKIP = could not be compared (note = reason); review these separately
"""


def scan(family=None):
    """All migrated definitions of the family, as Entries sorted by address."""
    if (family or FAMILY) == "mainwindow":
        return scan_mainwindow()
    entries = []
    files = sorted(glob.glob(os.path.join(vq.SRC, "*.cpp")) + glob.glob(os.path.join(vq.SRC, "*.h")))
    for path in files:
        found, _ = vq.find_definitions(path)
        rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
        for addr, qname, _lineno in found:
            cls = qname.rsplit("::", 1)[0] if "::" in qname else ""
            if cls.startswith("Vis") and cls not in EXCLUDED_CLASSES:
                entries.append(Entry(cls, int(addr, 16), addr.upper(), qname, rel, addr.upper(), None))
    entries.sort()
    return entries


def scan_mainwindow():
    """MainWindow definitions in main_window.cpp. Three address-comment styles exist:
    a standalone `// ADDR [client CL]` line above the definition, a trailing
    `{ // ADDR [client CL]` on the signature line, or the address as the first line
    INSIDE the body (`// 48d49b` + optional `// client: 491619`). Client addresses
    also come from the `(client: <va>)` pairs in main_window.h. Methods whose only
    address is a client one (e.g. RunModalLoopClient) are keyed by it. Inside an
    `#ifdef A2CLIENT` block a definition is client-keyed; inside the `#else` branch
    (A2SERVER) any client address is dropped."""
    client_map = {}
    header_map = {}  # method name -> (srv, cli) from header declarations; fallback for
    #                   cpp definitions that carry no local address comment (Proc_44c)
    with open(MAINWINDOW_H, encoding="utf-8", errors="replace") as fh:
        hdr_lines = fh.read().splitlines()
    # scope to the MainWindow class block: the header also declares dialog classes
    # (ServerControlDialog, ...) whose method names and addresses would collide
    start = next((i for i, l in enumerate(hdr_lines) if l.startswith("class MainWindow ")), None)
    end = next((i for i in range(start + 1, len(hdr_lines)) if hdr_lines[i] == "};"), len(hdr_lines)) \
        if start is not None else 0
    hdr = "\n".join(hdr_lines[start or 0:end or None])
    for m in re.finditer(r"//\s*([0-9A-Fa-f]{6})\s*\(client:\s*([0-9A-Fa-f]{6})\)", hdr):
        client_map[m.group(1).upper()] = m.group(2).upper()
    for line in hdr.splitlines():
        code, _, comment = line.partition("//")
        m = re.search(r"^\s*([0-9A-Fa-f]{6})\s*(?:\(client:\s*([0-9A-Fa-f]{6})\))?\s*$", comment)
        if not m or not code.rstrip().endswith(";"):
            continue
        n = re.search(r"(~?[A-Za-z_]\w*)\s*\(", code)
        if n:
            header_map[n.group(1)] = (m.group(1).upper(),
                                      m.group(2).upper() if m.group(2) else None)
    entries = []
    with open(MAINWINDOW_CPP, encoding="utf-8", errors="replace") as fh:
        lines = fh.read().splitlines()
    pending = None   # (srv|None, cli|None) from an address comment awaiting its definition
    build = []       # stack of "client" | "server" for the active #ifdef A2CLIENT branch

    def sweep_build(line):
        m = re.match(r"\s*#\s*(ifdef|ifndef|if|else|elif|endif)\b\s*([A-Za-z_]\w*)?", line)
        if not m:
            return
        kind, macro = m.group(1), m.group(2)
        if kind in ("ifdef", "ifndef", "if"):
            if macro == "A2CLIENT":
                build.append("server" if kind == "ifndef" else "client")
            else:
                build.append("other")  # any other conditional nests opaquely
        elif kind in ("else", "elif"):
            if build:
                build[-1] = {"client": "server", "server": "client"}.get(build[-1], "other")
        elif kind == "endif":
            if build:
                build.pop()

    def addr_of(idx):
        """Address comment(s) as the first line(s) of the body: returns (srv, cli) or
        None. A lone `{` opening line is skipped (multi-line signatures)."""
        j = idx
        if j < len(lines) and lines[j].strip() == "{":
            j += 1
        found = []
        while j < len(lines) and (not lines[j].strip() or lines[j].lstrip().startswith("//")):
            m = ADDR_STANDALONE.match(lines[j])
            if m:
                found.append((m.group("srv").upper(), m.group("cli").upper() if m.group("cli") else None))
                if len(found) >= 2:
                    break
            else:
                m = ADDR_CLIENT_ONLY.match(lines[j])
                if m:
                    found.append((None, m.group("cli").upper()))
                    if len(found) >= 2:
                        break
            j += 1
        if not found:
            return None
        srv, cli = found[0]
        if len(found) == 2 and found[1][0] is None:
            cli = found[1][1]
        return srv, cli

    for i, raw in enumerate(lines):
        sweep_build(raw)
        m = ADDR_STANDALONE.match(raw)
        if m:
            srv, cli = m.group("srv").upper(), (m.group("cli").upper() if m.group("cli") else None)
            if cli:
                client_map[srv] = cli
            pending = (srv, cli)
            continue
        m = ADDR_CLIENT_ONLY.match(raw)
        if m:
            cli = m.group("cli").upper()
            if pending and pending[0]:
                client_map[pending[0]] = cli  # supplement under a def's addr comment
            else:
                pending = (None, cli)  # client-only definition marker
            continue
        m = DEF_TRAILING.match(raw)
        def_name = None
        srv = cli = None
        if m and (m.group("srv") or m.group("cli")):
            def_name = m.group(1)
            srv = m.group("srv").upper() if m.group("srv") else None
            cli = m.group("cli").upper() if m.group("cli") else None
        else:
            m = DEF_PLAIN.match(raw)
            if m:
                def_name = m.group(1)
                if pending is not None:
                    srv, cli = pending
                else:
                    body = addr_of(i + 1)
                    if body:
                        srv, cli = body
        if def_name is not None and srv is None and cli is None:
            fb = header_map.get(def_name)  # e.g. Proc_44c: address lives on the decl only
            if fb:
                srv, cli = fb
        if def_name is None or (srv is None and cli is None):
            if raw.strip() and not raw.lstrip().startswith("//"):
                pending = None  # a code line between the comment and the def breaks the link
            continue
        pending = None
        in_client = in_server = False
        for b in reversed(build):  # innermost whole-function A2CLIENT gate wins
            if b in ("client", "server"):
                in_client, in_server = b == "client", b == "server"
                break
        if in_client:
            srv = None  # client-build branch: the server address is not its ground truth
        elif not in_server:
            cli = cli or client_map.get(srv) if srv else cli
        if srv is None and cli is None:
            fb = header_map.get(def_name)  # e.g. Proc_44c: address lives on the decl only
            if fb:
                srv, cli = fb
                if in_client:
                    srv = None
        entries.append(_mw_entry(def_name, srv, cli))
    entries.sort(key=lambda e: e.addr)
    return entries


def _mw_entry(name, srv, cli):
    key = srv or cli
    return Entry("MainWindow", int(key, 16), key, "MainWindow::" + name,
                 "src/main_window.cpp", srv, cli)


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
    label = FAMILIES[FAMILY]["label"]
    if not todo:
        print("DONE: all %d %s functions checked (%d OK, %d FIX, %d SKIP)" % (
            len(entries), label,
            sum(1 for s, _, _ in done.values() if s == "OK"),
            sum(1 for s, _, _ in done.values() if s == "FIX"),
            sum(1 for s, _, _ in done.values() if s == "SKIP")))
        return
    e = todo[0]
    cli_tag = "  client: %s" % e.cli if e.cli else ""
    print("NEXT  %s  %s  (%s, %s)%s  [%d/%d done]" % (
        e.ADDR, e.qname, e.rel, e.cls, cli_tag, len(entries) - len(todo), len(entries)))
    if e.srv:
        print("  python tool/gh.py decompile %s" % e.srv)
        print("  python tool/gh.py disasm %s" % e.srv)
    if e.cli:
        print("  python tool/gh.py --client decompile %s" % e.cli)
        print("  python tool/gh.py --client disasm %s   (when the client side differs)" % e.cli)
    print("  -> fix + build + `Fix %s: ...` commit, then: ok %s --fixed --note \"...\"" % (e.qname, e.ADDR))
    print("  -> or if clean: ok %s --note \"trickiest point verified\"" % e.ADDR)


def cmd_status(_args):
    entries = scan()
    done = read_log()
    todo = pending(entries, done)
    by_class = {}
    for e in todo:
        by_class.setdefault(e.cls, []).append(e.qname)
    print("%s recheck: %d/%d done, %d pending" % (
        FAMILIES[FAMILY]["label"], len(entries) - len(todo), len(entries), len(todo)))
    counts = {"OK": 0, "FIX": 0, "SKIP": 0}
    for s, _, _ in done.values():
        if s in counts:
            counts[s] += 1
    print("logged: %d OK, %d FIX, %d SKIP" % (counts["OK"], counts["FIX"], counts["SKIP"]))
    if todo:
        print("next: %s %s (%s)" % (todo[0].ADDR, todo[0].qname, todo[0].cls))
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
    for e in todo:
        if args.cls and e.cls != args.cls:
            continue
        print("%s\t%s\t%s%s" % (e.ADDR, e.qname, e.rel, ("\tclient " + e.cli) if e.cli else ""))
        shown += 1
    if not shown:
        print("no pending items%s" % (" for class %s" % args.cls if args.cls else ""))


def cmd_ok(args):
    entries = {e.ADDR: e for e in scan()}
    key = args.addr.upper()
    if key not in entries:
        print("warning: %s is not in the current %s scan (renamed or removed?); logging anyway" % (
            key, FAMILIES[FAMILY]["label"]))
    qname = entries[key].qname if key in entries else args.addr
    status = "FIX" if args.fixed else "OK"
    append_log(status, key, qname, args.note, args.force, args.no_fold)


def cmd_skip(args):
    entries = {e.ADDR: e for e in scan()}
    key = args.addr.upper()
    qname = entries[key].qname if key in entries else args.addr
    append_log("SKIP", key, qname, args.note, args.force, args.no_fold)


def main():
    global FAMILY, LOG, LOG_REL
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--family", choices=sorted(FAMILIES), default=os.environ.get("A2_RECHECK_FAMILY", "vis"),
                    help="audit family: vis (default) or mainwindow; env A2_RECHECK_FAMILY")
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
    FAMILY = args.family
    LOG_REL = FAMILIES[FAMILY]["log_rel"]
    LOG = os.path.join(ROOT, LOG_REL)
    args.func(args)


if __name__ == "__main__":
    main()
