#!/usr/bin/env python
"""Push function prototypes from our C++ headers into Ghidra (via the daemon).

The headers are the source of truth: every method declaration whose address is
known - either from a trailing comment (`// 55e129`, `// sub_55D579`,
`//4a47a0 in asm`) or from its name (`sub_55ECFE`, `FUN_0055ee42`) - is applied
to Ghidra as name + class + calling convention + return/param types + names.

    python tool/ghidra_backport.py --dry-run                 # list what would be applied
    python tool/ghidra_backport.py --class QuestMap          # one class (after editing quest_map.h)
    python tool/ghidra_backport.py --addr 55ECFE             # one function
    python tool/ghidra_backport.py --file src/quest_map.h    # one header
    python tool/ghidra_backport.py                           # everything

Classes with no matching structure in Ghidra are skipped unless --create-classes.
Parameters whose type is unknown to Ghidra are reported and the function skipped
(create the type first with `gh.py struct create`, or use --create-types for
pointer-to-unknown-class parameters).
"""
import argparse
import glob
import json
import os
import re
import sys
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
PORT = 18812

ADDR_IN_NAME = re.compile(r"^(?:sub_([0-9A-Fa-f]{6})|FUN_00([0-9A-Fa-f]{6}))$")
ADDR_IN_COMMENT = re.compile(r"(?<![\w])(?:sub_|FUN_00|0x)?((?=[0-9a-fA-F]*\d)[0-9a-fA-F]{6})(?![\w])")
CLASS_RE = re.compile(r"^\s*(class|struct|namespace)\s+([A-Za-z_]\w*)\s*(?::[^{;]*)?\{")
CLASS_OPEN_RE = re.compile(r"^\s*(class|struct|namespace)\s+([A-Za-z_]\w*)\s*(?::[^{;]*)?$")
DECL_RE = re.compile(
    r"^\s*(?P<kw>(?:(?:virtual|static|inline|explicit)\s+)*)"
    r"(?P<ret>(?:[A-Za-z_][\w:]*(?:<[^;{}]*?>)?[\s\*&]+(?:const\s*[\*&]*\s*)?)*?)"
    r"(?P<name>~?[A-Za-z_]\w*)\s*\((?P<params>[^;{}]*)\)\s*(?P<const>const)?\s*(?:(?:override|final)\s*)*(?:=\s*(?:0|default|delete))?\s*(?P<end>[;{])"
    r"(?P<tail>.*)$")
SKIP_NAMES = {"if", "for", "while", "switch", "return", "sizeof", "ASSERT_OFFSET", "ASSERT_SIZE", "static_assert"}


def split_params(s):
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch in "<(":
            depth += 1
        elif ch in ">)":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur)
    return [p.strip() for p in out if p.strip()]


def norm_type(t):
    if "(" in t:  # function pointer
        return "void *"
    t = re.sub(r"\b(const|class|struct|enum|AFXAPI|PASCAL|WINAPI|CALLBACK|__cdecl|__stdcall|__fastcall|__thiscall)\b", " ", t)
    t = t.replace("&", "*")
    t = re.sub(r"\s+", " ", t).strip()
    t = re.sub(r"\s*\*\s*", " *", t).strip()
    return t


def parse_param(p):
    p = p.split("=")[0].strip()
    if p in ("void", "..."):
        return None
    m = re.fullmatch(r"(.+?[\s\*&>])\s*([A-Za-z_]\w*)\s*(\[[^\]]*\])?", p)
    if m and not re.fullmatch(r"(unsigned|signed|const|char|short|int|long)", m.group(2)) and \
            not m.group(1).strip().endswith(("<", ",")):
        t = norm_type(m.group(1)) + (" *" if m.group(3) else "")
        return t, m.group(2)
    return norm_type(p), None


def parse_headers(files):
    """Yield dicts: file, line, cls, name, ret, params[(type,name)], addr, virtual, static."""
    for path in files:
        try:
            lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
        except OSError:
            continue
        stack = []  # (scope_name, brace_depth_at_open, is_namespace)
        pending = None  # scope seen, waiting for its '{' on a following line
        depth = 0
        for ln, raw in enumerate(lines, 1):
            line = raw.split("//")[0]
            cm = CLASS_RE.match(raw)
            if cm:
                stack.append((cm.group(2), depth, cm.group(1) == "namespace"))
            elif pending and line.lstrip().startswith("{"):
                stack.append((pending[0], depth, pending[1]))
                pending = None
            else:
                om = CLASS_OPEN_RE.match(line.rstrip())
                pending = (om.group(2), om.group(1) == "namespace") if om else None
            dm = DECL_RE.match(raw)
            if dm and dm.group("name") not in SKIP_NAMES and not re.match(r"\s*(typedef|using|template|#|extern)", raw):
                cls, is_ns = (stack[-1][0], stack[-1][2]) if stack else (None, True)
                name = dm.group("name")
                ret = norm_type(dm.group("ret"))
                kw = dm.group("kw")
                is_ctor = name == cls
                is_dtor = cls is not None and name == "~" + cls
                if not ret and not (is_ctor or is_dtor):
                    pass
                elif stack and depth > stack[-1][1] + 1:
                    pass  # inside a method body
                else:
                    addr = None
                    am = ADDR_IN_NAME.match(name)
                    if am:
                        addr = (am.group(1) or am.group(2)).upper()
                    else:
                        cm2 = ADDR_IN_COMMENT.search(dm.group("tail"))
                        if cm2:
                            addr = cm2.group(1).upper()
                    params = [x for x in (parse_param(p) for p in split_params(dm.group("params"))) if x]
                    cc = None
                    ccm = re.search(r"__(cdecl|stdcall|fastcall|thiscall)\b", dm.group("ret") + " " + kw)
                    if ccm:
                        cc = "__" + ccm.group(1)
                    elif is_ns or "static" in kw:
                        cc = "__cdecl"
                    else:
                        cc = "__thiscall"
                    yield {"file": os.path.relpath(path, ROOT), "line": ln, "cls": cls, "name": name,
                           "ret": (cls + " *") if is_ctor else ("void" if is_dtor else ret),
                           "params": params, "addr": addr, "virtual": "virtual" in kw, "cc": cc,
                           "plain_ns": is_ns, "ctor": is_ctor, "dtor": is_dtor}
            depth += line.count("{") - line.count("}")
            while stack and depth <= stack[-1][1]:
                stack.pop()


def parse_cpp_addrs(files):
    """Map (cls, name) -> addr from '// ADDR' lines directly above definitions."""
    out = {}
    defn = re.compile(r"^[\w:<>\*&\s,~]*?\b([A-Za-z_]\w*)::(~?[A-Za-z_]\w*|operator\S+)\s*\(")
    for path in files:
        try:
            lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
        except OSError:
            continue
        for i, raw in enumerate(lines[:-1]):
            m = re.fullmatch(r"\s*//\s*(?:0x)?([0-9A-Fa-f]{6})\b.*", raw)
            if not m:
                continue
            for j in range(i + 1, min(i + 3, len(lines))):
                dm = defn.match(lines[j])
                if dm:
                    out.setdefault((dm.group(1), dm.group(2)), m.group(1).upper())
                    break
    return out


def call(op, args):
    data = json.dumps({"op": op, "args": args}).encode("utf-8")
    req = urllib.request.Request(f"http://127.0.0.1:{PORT}/", data=data, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=3600) as r:
        resp = json.loads(r.read().decode("utf-8"))
    if not resp.get("ok"):
        sys.exit("ERROR: " + resp.get("error", "?"))
    return resp["result"]


def main():
    global PORT
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--class", dest="cls", help="only this class")
    ap.add_argument("--addr", help="only this address (hex)")
    ap.add_argument("--file", help="only declarations from this header")
    ap.add_argument("--create-classes", action="store_true", help="create empty structs for unknown classes")
    ap.add_argument("--create-types", action="store_true", help="create placeholder structs for unknown pointer types")
    ap.add_argument("--port", type=int, default=PORT)
    a = ap.parse_args()
    PORT = a.port

    headers = [a.file] if a.file else sorted(glob.glob(os.path.join(SRC, "*.h")))
    cpp_addrs = parse_cpp_addrs(sorted(glob.glob(os.path.join(SRC, "*.cpp"))))
    decls = list(parse_headers(headers))
    for d in decls:
        if not d["addr"]:
            d["addr"] = cpp_addrs.get((d["cls"], d["name"]))

    seen, todo, skipped = set(), [], []
    for d in decls:
        if a.cls and d["cls"] != a.cls:
            continue
        if a.addr and (d["addr"] or "").upper() != a.addr.upper().removeprefix("0X"):
            continue
        if not d["addr"]:
            skipped.append((d, "no address"))
            continue
        if d["addr"] in seen:
            skipped.append((d, "duplicate address"))
            continue
        if d["name"].startswith("operator"):
            skipped.append((d, "operator"))
            continue
        seen.add(d["addr"])
        todo.append(d)

    print(f"{len(decls)} declarations parsed, {len(todo)} with addresses to apply, {len(skipped)} skipped")
    if a.dry_run:
        for d in todo:
            ps = ", ".join(f"{t} {n}" if n else t for t, n in d["params"])
            scope = (d["cls"] + "::") if d["cls"] else ""
            print(f"  {d['addr']}  {d['ret']} {d['cc']} {scope}{d['name']}({ps})  [{d['file']}:{d['line']}]")
        for d, why in skipped:
            if why != "no address":
                print(f"  SKIP ({why}) {d['cls']}::{d['name']} [{d['file']}:{d['line']}]")
        return

    ops = []
    for d in todo:
        ops.append({"op": "set-proto", "args": {
            "target": d["addr"], "ret": d["ret"],
            "params": [f"{t} {n}" if n else t for t, n in d["params"]],
            "cc": d["cc"], "cls": d["cls"] or "", "name": d["name"], "plain_ns": d["plain_ns"],
            "keep_better": True, "split": True,
            "create": a.create_classes or a.create_types}})
        if d["virtual"]:
            ops.append({"op": "set-comment", "args": {"target": d["addr"], "text": "virtual"}})
    # chunks keep each save reasonably sized and let progress show
    ok = err = 0
    for i in range(0, len(ops), 200):
        res = call("batch", {"ops": ops[i:i + 200]})
        for line in res.splitlines():
            if line.startswith("ERR"):
                err += 1
                print(line)
            else:
                ok += 1
    print(f"applied {ok} ops, {err} errors")


if __name__ == "__main__":
    main()
