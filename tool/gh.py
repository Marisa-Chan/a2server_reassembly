#!/usr/bin/env python
"""CLI client for the Ghidra daemon (tool/ghidra_daemon.py or tool/ghidra_gui_server.py).

One daemon serves BOTH programs: pass --client / --server (or --program NAME)
BEFORE the command. GHIDRA_PROGRAM=client keeps the selection sticky for a whole
shell session; an explicit flag always wins. Output is tagged [server]/[client].

Addresses can be given as 55ECFE, 0x55ECFE, sub_55ECFE, FUN_0055ecfe, or a symbol
name (QuestMap::sub_55ECFE, or a mangled name). IDA data names (off_60AF78,
dword_6CDB3C, ...) also work.

Examples:
  python tool/gh.py info 55ECFE
  python tool/gh.py --client context 4e2c33     # allods2.exe instead of a2serv6.exe_new
  python tool/gh.py decompile 55ECFE
  python tool/gh.py global off_60AF78 --lines 20
  python tool/gh.py set-mangled 55ECFE "?sub_55ECFE@QuestMap@@QAEXH@Z" player_id
  python tool/gh.py set-proto 55ECFE void --class QuestMap -p "int32_t player_id"
  python tool/gh.py set-var 55ECFE local_10 --name assoc --type "CAssoc<int,p_Quest> *"
  python tool/gh.py struct show Unit
  python tool/gh.py struct set-field Unit 0x8 "Unit *" owner
"""
import argparse
import json
import os
import sys
import urllib.error
import urllib.request

DEFAULT_PORT = 18812  # keep in sync with ghidra_ops.DEFAULT_PORT

HELP = """\
PROGRAM SELECTION (one daemon serves both a2serv6.exe_new and allods2.exe; flags go BEFORE the command)
  --client                          talk to allods2.exe
  --server                          talk to a2serv6.exe_new (default)
  --program NAME                    server/client/a2serv6.exe_new/allods2.exe
  GHIDRA_PROGRAM env var            sticky default for the shell session (explicit flags win)

READ
  context ADDR                      info + callee prototypes + decompiled C in one go (use this first)
  info ADDR                         name, class, prototype, params, locals, callers
  decompile ADDR                    decompiled C (typed, from live Ghidra project)
  disasm ADDR                       Ghidra disassembly listing
  xrefs ADDR                        who references this address
  callees ADDR                      functions called by this function
  global ADDR [--lines N]           show a global variable / vtable: data at ADDR, then N following data items
  find REGEX                        search function/label/class names
  demangle MANGLED                  show what a mangled name means (validates it)
WRITE (functions)
  set-mangled ADDR MANGLED [NAME..] name + class + full signature from an MSVC-mangled name;
                                    optional parameter names (in order, without `this`)
  set-proto ADDR RET [-p "T name"]... explicit signature; --class C puts it in class C (adds `this`),
                                    --cc __thiscall|__cdecl|__stdcall|__fastcall, --name N, --create
  rename ADDR NAME [--class C]      rename (and optionally move into class C)
  set-class ADDR CLASS              move function into class CLASS (keeps name)
  set-var ADDR VAR --name N --type T   rename/retype a param or local (decompiler names)
  comment ADDR TEXT                 set function plate comment
WRITE (structs)
  struct show NAME                  fields with offsets
  struct list [REGEX]
  struct create NAME SIZE
  struct resize NAME SIZE
  struct set-field NAME OFFSET TYPE [FIELD] [--comment C]
  struct clear-field NAME OFFSET
  struct rename NAME NEW
MISC
  status | save-all | save | undo
  python "CODE" | python -f FILE    run Python inside the daemon (program, ctx, programs in scope)
Types: Ghidra names (int, uint, short, ushort, char, uchar, bool, float, double, void, Unit *)
       or C++ names (int32_t, uint8_t, ...). Unknown struct pointers need --create or `struct create`.
"""


def call(op, args, port, program=None):
    payload = {"op": op, "args": args}
    if program:
        payload["program"] = program
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(f"http://127.0.0.1:{port}/", data=data, headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=600) as r:
            resp = json.loads(r.read().decode("utf-8"))
    except urllib.error.URLError as e:
        whom = f" for program '{program}'" if program else ""
        sys.exit(f"ERROR: cannot reach Ghidra daemon on port {port}{whom} ({e.reason}). Start it: python tool/ghidra_daemon.py")
    if program and "program" not in resp:
        sys.exit(f"ERROR: daemon on port {port} predates program routing; restart it: python tool/ghidra_daemon.py")
    # the tag shows which binary actually answered - a wrong GHIDRA_PROGRAM is visible immediately
    name = resp.get("program") or program or "server"
    if not resp.get("ok"):
        msg = f"ERROR [{name}]: " + resp.get("error", "?")
        if os.environ.get("GHIDRA_CLI_TRACE") and resp.get("trace"):
            msg += "\n" + resp["trace"]
        sys.exit(msg)
    lines = str(resp["result"]).split("\n")
    print("\n".join([f"[{name}] " + lines[0]] + lines[1:]))


def main():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument("--port", type=int, default=DEFAULT_PORT)
    sel = ap.add_mutually_exclusive_group()
    sel.add_argument("--client", action="store_true", help="talk to allods2.exe (the client program)")
    sel.add_argument("--server", action="store_true", help="talk to a2serv6.exe_new (the server program, default)")
    sel.add_argument("--program", help="program by name: server/client/a2serv6.exe_new/allods2.exe")
    ap.add_argument("cmd", nargs="?")
    ap.add_argument("rest", nargs=argparse.REMAINDER)
    top = ap.parse_args()
    cmd, rest, port = top.cmd, top.rest, top.port
    program = "client" if top.client else "server" if top.server else (top.program or os.environ.get("GHIDRA_PROGRAM") or None)
    if cmd in (None, "help", "-h", "--help"):
        print(HELP)
        return

    def sub(**kw):
        p = argparse.ArgumentParser(prog=f"gh.py {cmd}")
        for k, v in kw.items():
            p.add_argument(*v[0], **v[1]) if isinstance(v, tuple) else p.add_argument(k, **v)
        return p.parse_args(rest)

    if cmd in ("context", "info", "decompile", "disasm", "xrefs", "callees"):
        a = sub(target={})
        call(cmd, {"target": a.target}, port, program)
    elif cmd == "find":
        a = sub(pattern={}, limit=(["--limit"], {"type": int, "default": 50}))
        call(cmd, {"pattern": a.pattern, "limit": a.limit}, port, program)
    elif cmd == "global":
        a = sub(target={}, lines=(["--lines", "-n"], {"type": int, "default": 10}))
        call(cmd, {"target": a.target, "lines": a.lines}, port, program)
    elif cmd == "demangle":
        a = sub(mangled={})
        call(cmd, {"mangled": a.mangled}, port, program)
    elif cmd == "set-mangled":
        a = sub(target={}, mangled={}, names={"nargs": "*"})
        call(cmd, {"target": a.target, "mangled": a.mangled, "names": a.names}, port, program)
    elif cmd == "set-proto":
        a = sub(target={}, ret={}, params=(["-p", "--param"], {"action": "append", "default": [], "dest": "params"}),
                cc=(["--cc"], {}), cls=(["--class"], {"dest": "cls"}), name=(["--name"], {}),
                create=(["--create"], {"action": "store_true"}))
        call(cmd, {"target": a.target, "ret": a.ret, "params": a.params, "cc": a.cc, "cls": a.cls,
                   "name": a.name, "create": a.create}, port, program)
    elif cmd == "rename":
        a = sub(target={}, name={}, cls=(["--class"], {"dest": "cls"}))
        call(cmd, {"target": a.target, "name": a.name, "cls": a.cls}, port, program)
    elif cmd == "set-class":
        a = sub(target={}, cls={})
        call(cmd, {"target": a.target, "cls": a.cls}, port, program)
    elif cmd == "set-var":
        a = sub(target={}, var={}, name=(["--name"], {}), type=(["--type"], {}),
                create=(["--create"], {"action": "store_true"}))
        call(cmd, {"target": a.target, "var": a.var, "name": a.name, "type": a.type, "create": a.create}, port, program)
    elif cmd == "comment":
        a = sub(target={}, text={"nargs": "+"})
        call("set-comment", {"target": a.target, "text": " ".join(a.text)}, port, program)
    elif cmd == "struct":
        if not rest:
            sys.exit(HELP)
        scmd, rest = rest[0], rest[1:]
        cmd = f"struct {scmd}"
        if scmd == "show":
            a = sub(name={})
            call("struct-show", {"name": a.name}, port, program)
        elif scmd == "list":
            a = sub(pattern={"nargs": "?", "default": "."})
            call("struct-list", {"pattern": a.pattern}, port, program)
        elif scmd == "create":
            a = sub(name={}, size={}, category=(["--category"], {"default": "/Game"}))
            call("struct-create", {"name": a.name, "size": a.size, "category": a.category}, port, program)
        elif scmd == "resize":
            a = sub(name={}, size={})
            call("struct-resize", {"name": a.name, "size": a.size}, port, program)
        elif scmd == "set-field":
            a = sub(name={}, offset={}, type={}, field={"nargs": "?"}, comment=(["--comment"], {}),
                    create=(["--create"], {"action": "store_true"}))
            call("struct-set-field", {"name": a.name, "offset": a.offset, "type": a.type, "field": a.field,
                                      "comment": a.comment, "create": a.create}, port, program)
        elif scmd == "clear-field":
            a = sub(name={}, offset={})
            call("struct-clear-field", {"name": a.name, "offset": a.offset}, port, program)
        elif scmd == "rename":
            a = sub(name={}, new_name={})
            call("struct-rename", {"name": a.name, "new_name": a.new_name}, port, program)
        else:
            sys.exit(f"unknown struct subcommand '{scmd}'\n" + HELP)
    elif cmd in ("status", "save", "save-all", "undo", "reload", "shutdown"):
        call(cmd, {}, port, program)
    elif cmd == "python":
        a = sub(code={"nargs": "?"}, file=(["-f", "--file"], {}))
        code = open(a.file, encoding="utf-8").read() if a.file else a.code
        if not code:
            sys.exit("give CODE or -f FILE")
        call(cmd, {"code": code}, port, program)
    else:
        sys.exit(f"unknown command '{cmd}'\n" + HELP)


if __name__ == "__main__":
    main()
