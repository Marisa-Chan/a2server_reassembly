#!/usr/bin/env python
"""Push class/struct layouts from our C++ headers into Ghidra structures (via the daemon).

Field offsets and sizes are not guessed: a small C++ program is generated that
`#include`s the headers and prints `offsetof`/`sizeof` for every data member, compiled
with the same MSVC x86 toolset as the project. Base-class members are flattened into
the derived class (offsetof works through inheritance), so `this->token_pos` resolves
directly in the decompiler.

    python tool/ghidra_sync_structs.py --dry-run                 # list what would change
    python tool/ghidra_sync_structs.py --class Unit --class Token
    python tool/ghidra_sync_structs.py --file src/unit.h
    python tool/ghidra_sync_structs.py                           # every class in src/*.h

Merge policy (see ghidra_ops.op_struct_apply):
  - Ghidra structs are created if missing, grown to sizeof(); shrunk only if the header has ASSERT_SIZE.
  - A header field replaces undefined bytes, or a Ghidra field occupying exactly the same slot.
  - Placeholder header names (fieldN_0xXX, unk*, pad*) never overwrite a real Ghidra name/type.
  - Types unknown to Ghidra fall back to the existing component type of the same size, else undefined.
  - Offset 0 of polymorphic classes (vtable pointer) is left alone.
"""
import argparse
import glob
import importlib.util
import json
import os
import re
import subprocess
import sys
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
BUILD = os.path.join(ROOT, "build")
PORT = 18812
VSDEVCMD = r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
DEFINES = ["WIN32", "_WINDOWS", "FKG_FORCED_USAGE", "_USE_32BIT_TIME_T", "NDEBUG", "_CONSOLE",
           "_CRT_SECURE_NO_WARNINGS", "A2SERVER_PATCH", "A2SERVER_TANGAR_HAT"]
# headers that are not standalone-includable or irrelevant for layouts
EXCLUDE_HEADERS = {"afxmsg_.h", "dplay.h", "dplobby.h", "music.h", "mfc_templ_impl.h"}

_spec = importlib.util.spec_from_file_location("ghidra_backport", os.path.join(ROOT, "tool", "ghidra_backport.py"))
bp = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(bp)

CLASS_RE = re.compile(r"^\s*(class|struct)\s+([A-Za-z_]\w*)\s*(?::\s*([^{;]*))?\s*(\{)?\s*$")
FIELD_RE = re.compile(
    r"^\s*(?P<type>(?:[A-Za-z_][\w:]*(?:<[^;]*?>)?[\s\*&]+)+?)\s*"
    r"(?P<decls>\**\s*[A-Za-z_]\w*(?:\s*\[[^\]]+\])*(?:\s*=[^,;]*)?(?:\s*,\s*\**\s*[A-Za-z_]\w*(?:\s*\[[^\]]+\])*(?:\s*=[^,;]*)?)*)\s*;")
SKIP_FIELD_LINE = re.compile(r"^\s*(static|typedef|using|friend|enum|return|template|public|private|protected|#|//|virtual|extern|operator)\b")
PLACEHOLDER_FIELD = re.compile(r"^(field\d*_?(0x|x)?[0-9a-fA-F]+|unk\w*|unknown\w*|pad\w*|_pad\w*|padding\w*|reserved\w*|gap\w*|dummy\w*)$", re.I)


def parse_classes(files):
    """Return {name: {file, line, bases:[...], fields:[(type, name, dims)], polymorphic, assert_size}}."""
    classes = {}
    for path in files:
        try:
            lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
        except OSError:
            continue
        text = "\n".join(lines)
        asserted = {m.group(1): int(m.group(2), 0) for m in re.finditer(r"ASSERT_SIZE\(\s*(\w+)\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*\)", text)}
        stack = []  # (name or None, depth_at_open, is_tracked_class)
        pending = None
        depth = 0
        prev_template = False
        for ln, raw in enumerate(lines, 1):
            line = raw.split("//")[0].rstrip()
            cm = CLASS_RE.match(line)
            if cm and not line.rstrip().endswith(";"):
                name, bases, brace = cm.group(2), cm.group(3) or "", cm.group(4)
                tracked = depth == 0 and not prev_template
                info = {"file": os.path.relpath(path, ROOT), "line": ln, "fields": [], "polymorphic": False,
                        "bases": [b.strip().split()[-1] for b in bases.split(",") if b.strip()],
                        "assert_size": asserted.get(name)}
                if brace:
                    stack.append((name, depth, tracked))
                    if tracked:
                        classes[name] = info
                else:
                    pending = (name, tracked, info)
            elif pending and line.lstrip().startswith("{"):
                name, tracked, info = pending
                stack.append((name, depth, tracked))
                if tracked:
                    classes[name] = info
                pending = None
            elif stack and stack[-1][2] and depth == stack[-1][1] + 1:
                cls = classes[stack[-1][0]]
                if re.search(r"\bvirtual\b", line) or "VTable" in raw:
                    cls["polymorphic"] = True
                if "(" not in line and ":" not in line.split("=")[0] and not SKIP_FIELD_LINE.match(line):
                    fm = FIELD_RE.match(line)
                    if fm:
                        base_type = bp.norm_type(fm.group("type"))
                        for decl in bp.split_params(fm.group("decls")):
                            decl = decl.split("=")[0].strip()
                            dm = re.fullmatch(r"(\**)\s*([A-Za-z_]\w*)\s*((?:\[[^\]]+\])*)", decl)
                            if dm:
                                t = base_type + (" *" * len(dm.group(1)))
                                cls["fields"].append((t, dm.group(2), dm.group(3), ln))
            prev_template = bool(re.match(r"^\s*template\s*<", line))
            depth += line.count("{") - line.count("}")
            while stack and depth <= stack[-1][1]:
                stack.pop()
    return classes


def all_fields(name, classes, seen=None):
    """Own + inherited fields (base first), skipping bases we don't know."""
    seen = seen or set()
    if name in seen or name not in classes:
        return []
    seen.add(name)
    out = []
    for b in classes[name]["bases"]:
        out.extend(all_fields(b, classes, seen))
    out.extend(classes[name]["fields"])
    return out


def compile_dump(selected, classes, headers):
    """Generate, compile and run the offsetof dump for `selected` (base lookups use `classes`).
    Returns {cls: {"size": n, "fields": {fname: (off, size)}}}.
    Headers that fail to compile standalone are dropped (reported) and the compile retried."""
    os.makedirs(BUILD, exist_ok=True)
    cpp = os.path.join(BUILD, "struct_dump.cpp")
    exe = os.path.join(BUILD, "struct_dump.exe")
    headers = list(headers)
    for attempt in range(12):
        _write_dump_source(cpp, selected, classes, headers)
        cmd = (f'"{VSDEVCMD}" -arch=x86 -no_logo && cl /nologo /std:c++17 /EHsc /W0 /MD /I "{SRC}" /I "{SRC}\\mfc\\include" '
               + " ".join(f"/D{d}" for d in DEFINES) + f' /Fo"{BUILD}\\struct_dump.obj" /Fe"{exe}" "{cpp}"')
        # cmd.exe strips the outer quotes of a /c command that starts with a quote, so wrap it once more
        r = subprocess.run(f'cmd /c "{cmd}"', capture_output=True, text=True, cwd=BUILD)
        if r.returncode == 0:
            break
        errs = [l for l in (r.stdout + r.stderr).splitlines() if "error" in l] or (r.stdout + r.stderr).splitlines()[-10:]
        bad = {os.path.basename(m.group(1)) for l in errs for m in [re.search(r"src[\\/]([\w.]+\.h)\(", l)] if m}
        bad = {b for b in bad if any(os.path.basename(h) == b for h in headers)}
        if not bad:
            sys.exit("struct_dump.cpp failed to compile:\n" + "\n".join(errs[:40]) + f"\n(see {cpp})")
        print(f"  excluding header(s) that do not compile standalone: {', '.join(sorted(bad))}")
        headers = [h for h in headers if os.path.basename(h) not in bad]
    else:
        sys.exit("struct_dump.cpp: too many retries")
    out = subprocess.run([exe], capture_output=True, text=True).stdout
    dump = {}
    for line in out.splitlines():
        p = line.split()
        if p[0] == "CLASS":
            dump.setdefault(p[1], {"size": 0, "fields": {}})["size"] = int(p[2])
        elif p[0] == "FIELD":
            dump.setdefault(p[1], {"size": 0, "fields": {}})["fields"][p[2]] = (int(p[3]), int(p[4]))
    return dump


def _write_dump_source(cpp, selected, classes, headers):
    with open(cpp, "w", encoding="utf-8") as fh:
        fh.write("#define _ALLOW_KEYWORD_MACROS\n#define private public\n#define protected public\n#include <cstddef>\n#include <cstdio>\n")
        fh.write("#pragma warning(disable: 4267 4244 4996)\n")
        for h in headers:
            fh.write(f'#include "{os.path.relpath(h, BUILD).replace(os.sep, "/")}"\n')
        fh.write("int main() {\n")
        for name, info in selected.items():
            fh.write(f'  printf("CLASS {name} %u\\n", (unsigned)sizeof({name}));\n')
            for t, f, dims, ln in all_fields(name, classes):
                fh.write(f'  printf("FIELD {name} {f} %u %u\\n", (unsigned)offsetof({name}, {f}), (unsigned)sizeof((({name}*)0)->{f}));\n')
        fh.write("  return 0;\n}\n")


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
    ap.add_argument("--class", dest="cls", action="append", help="only these classes (repeatable)")
    ap.add_argument("--file", help="only classes declared in this header")
    ap.add_argument("--port", type=int, default=PORT)
    a = ap.parse_args()
    PORT = a.port

    headers = sorted(h for h in glob.glob(os.path.join(SRC, "*.h")) if os.path.basename(h) not in EXCLUDE_HEADERS)
    classes = parse_classes(headers)
    # compile-time errors would stop everything, so keep only classes that have fields
    classes = {n: c for n, c in classes.items() if all_fields(n, classes)}
    selected = dict(classes)
    if a.file:
        want = os.path.relpath(os.path.abspath(a.file), ROOT)
        selected = {n: c for n, c in classes.items() if c["file"] == want}
    if a.cls:
        missing = [c for c in a.cls if c not in classes]
        if missing:
            sys.exit("unknown class(es) in headers: " + ", ".join(missing))
        selected = {n: classes[n] for n in a.cls}
    if not selected:
        sys.exit("nothing selected")

    dump = compile_dump(selected, classes, headers)
    ops = []
    for name, info in selected.items():
        d = dump.get(name)
        if not d:
            print(f"  {name}: not in dump (compile skipped?)")
            continue
        fields = []
        for t, f, dims, ln in all_fields(name, classes):
            if f not in d["fields"]:
                continue
            off, size = d["fields"][f]
            fields.append({"offset": off, "size": size, "type": t + dims, "name": f,
                           "placeholder": bool(PLACEHOLDER_FIELD.match(f))})
        ops.append({"op": "struct-apply", "args": {
            "name": name, "size": d["size"], "fields": fields,
            "shrink": info["assert_size"] is not None, "polymorphic": info["polymorphic"]}})
        if a.dry_run:
            print(f"{name}  sizeof=0x{d['size']:X}  polymorphic={info['polymorphic']}  assert_size={info['assert_size']}  [{info['file']}:{info['line']}]")
            for fl in fields:
                print(f"    0x{fl['offset']:04X} {fl['size']:4}  {fl['type']:28} {fl['name']}")
    if a.dry_run:
        return
    # pass 1: sizes only (so by-value member structs resolve with the right size in pass 2)
    size_ops = [{"op": "struct-apply", "args": {**o["args"], "fields": None}} for o in ops]
    for line in call("batch", {"ops": size_ops}).splitlines():
        if line.startswith("ERR"):
            print(line)
    for i in range(0, len(ops), 100):
        for line in call("batch", {"ops": ops[i:i + 100]}).splitlines():
            print(line if line.startswith("ERR") else line.split(" -> ", 1)[-1])


if __name__ == "__main__":
    main()
