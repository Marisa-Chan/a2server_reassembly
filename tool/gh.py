#!/usr/bin/env python
"""CLI client for the Ghidra daemon (tool/ghidra_daemon.py or tool/ghidra_gui_server.py).

Addresses can be given as 55ECFE, 0x55ECFE, sub_55ECFE, FUN_0055ecfe, or a symbol
name (QuestMap::sub_55ECFE, or a mangled name).

Examples:
  python tool/gh.py info 55ECFE
  python tool/gh.py decompile 55ECFE
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
READ
  context ADDR                      info + callee prototypes + decompiled C in one go (use this first)
  info ADDR                         name, class, prototype, params, locals, callers
  decompile ADDR                    decompiled C (typed, from live Ghidra project)
  disasm ADDR                       Ghidra disassembly listing
  xrefs ADDR                        who references this address
  callees ADDR                      functions called by this function
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
  status | save | undo
  python "CODE" | python -f FILE    run Python inside the daemon (program, ctx in scope)
Types: Ghidra names (int, uint, short, ushort, char, uchar, bool, float, double, void, Unit *)
       or C++ names (int32_t, uint8_t, ...). Unknown struct pointers need --create or `struct create`.
"""


def call(op, args, port):
    data = json.dumps({"op": op, "args": args}).encode("utf-8")
    req = urllib.request.Request(f"http://127.0.0.1:{port}/", data=data, headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=600) as r:
            resp = json.loads(r.read().decode("utf-8"))
    except urllib.error.URLError as e:
        sys.exit(f"ERROR: cannot reach Ghidra daemon on port {port} ({e.reason}). Start it: python tool/ghidra_daemon.py")
    if not resp.get("ok"):
        msg = "ERROR: " + resp.get("error", "?")
        if os.environ.get("GHIDRA_CLI_TRACE") and resp.get("trace"):
            msg += "\n" + resp["trace"]
        sys.exit(msg)
    print(resp["result"])


def main():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument("--port", type=int, default=DEFAULT_PORT)
    ap.add_argument("cmd", nargs="?")
    ap.add_argument("rest", nargs=argparse.REMAINDER)
    top = ap.parse_args()
    cmd, rest, port = top.cmd, top.rest, top.port
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
        call(cmd, {"target": a.target}, port)
    elif cmd == "find":
        a = sub(pattern={}, limit=(["--limit"], {"type": int, "default": 50}))
        call(cmd, {"pattern": a.pattern, "limit": a.limit}, port)
    elif cmd == "demangle":
        a = sub(mangled={})
        call(cmd, {"mangled": a.mangled}, port)
    elif cmd == "set-mangled":
        a = sub(target={}, mangled={}, names={"nargs": "*"})
        call(cmd, {"target": a.target, "mangled": a.mangled, "names": a.names}, port)
    elif cmd == "set-proto":
        a = sub(target={}, ret={}, params=(["-p", "--param"], {"action": "append", "default": [], "dest": "params"}),
                cc=(["--cc"], {}), cls=(["--class"], {"dest": "cls"}), name=(["--name"], {}),
                create=(["--create"], {"action": "store_true"}))
        call(cmd, {"target": a.target, "ret": a.ret, "params": a.params, "cc": a.cc, "cls": a.cls,
                   "name": a.name, "create": a.create}, port)
    elif cmd == "rename":
        a = sub(target={}, name={}, cls=(["--class"], {"dest": "cls"}))
        call(cmd, {"target": a.target, "name": a.name, "cls": a.cls}, port)
    elif cmd == "set-class":
        a = sub(target={}, cls={})
        call(cmd, {"target": a.target, "cls": a.cls}, port)
    elif cmd == "set-var":
        a = sub(target={}, var={}, name=(["--name"], {}), type=(["--type"], {}),
                create=(["--create"], {"action": "store_true"}))
        call(cmd, {"target": a.target, "var": a.var, "name": a.name, "type": a.type, "create": a.create}, port)
    elif cmd == "comment":
        a = sub(target={}, text={"nargs": "+"})
        call("set-comment", {"target": a.target, "text": " ".join(a.text)}, port)
    elif cmd == "struct":
        if not rest:
            sys.exit(HELP)
        scmd, rest = rest[0], rest[1:]
        cmd = f"struct {scmd}"
        if scmd == "show":
            a = sub(name={})
            call("struct-show", {"name": a.name}, port)
        elif scmd == "list":
            a = sub(pattern={"nargs": "?", "default": "."})
            call("struct-list", {"pattern": a.pattern}, port)
        elif scmd == "create":
            a = sub(name={}, size={}, category=(["--category"], {"default": "/Game"}))
            call("struct-create", {"name": a.name, "size": a.size, "category": a.category}, port)
        elif scmd == "resize":
            a = sub(name={}, size={})
            call("struct-resize", {"name": a.name, "size": a.size}, port)
        elif scmd == "set-field":
            a = sub(name={}, offset={}, type={}, field={"nargs": "?"}, comment=(["--comment"], {}),
                    create=(["--create"], {"action": "store_true"}))
            call("struct-set-field", {"name": a.name, "offset": a.offset, "type": a.type, "field": a.field,
                                      "comment": a.comment, "create": a.create}, port)
        elif scmd == "clear-field":
            a = sub(name={}, offset={})
            call("struct-clear-field", {"name": a.name, "offset": a.offset}, port)
        elif scmd == "rename":
            a = sub(name={}, new_name={})
            call("struct-rename", {"name": a.name, "new_name": a.new_name}, port)
        else:
            sys.exit(f"unknown struct subcommand '{scmd}'\n" + HELP)
    elif cmd in ("status", "save", "undo", "reload", "shutdown"):
        call(cmd, {}, port)
    elif cmd == "python":
        a = sub(code={"nargs": "?"}, file=(["-f", "--file"], {}))
        code = open(a.file, encoding="utf-8").read() if a.file else a.code
        if not code:
            sys.exit("give CODE or -f FILE")
        call(cmd, {"code": code}, port)
    else:
        sys.exit(f"unknown command '{cmd}'\n" + HELP)


if __name__ == "__main__":
    main()
