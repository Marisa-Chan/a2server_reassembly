#!/usr/bin/env python
"""Push class/struct layouts (and vtables) from our C++ headers into Ghidra (via the daemon).

Nothing is guessed: a generated C++ program `#include`s the headers and is compiled with the
project's MSVC x86 toolset. `offsetof`/`sizeof` give field offsets; `/d1reportAllClassLayout`
gives base-class offsets, vfptr placement and the exact vtable slot order (overrides resolved).

Resulting Ghidra model (mirrors the C++ hierarchy):
  - `Derived` starts with member `_` of type `Base` (at the base-class offset MSVC reports), so
    inherited fields read as `this->_.hp` and `(Unit *)this` casts disappear from base-method calls.
  - Each hierarchy root (first polymorphic class below CObject, e.g. Token, CVisualObject) gets ONE
    `{Root}_VTable` (category /Game/VTables) holding the union of all descendants' slots, typed as
    __thiscall function pointers with signatures from the headers. Where sibling classes put different
    methods in the same slot (7 slots in the game hierarchies), that slot is a union of the candidates
    and the field comment lists them. The root's offset 0 is `{Root}_VTable *vptr`; descendants reach
    it through `_`, so `(*this->_.vptr->VMethod25)(this)` is fully named everywhere.
  - `--flat` instead flattens inherited fields into each struct and gives every class its own vtable
    (the old model; field access is shorter, base-method calls carry casts).

    python tool/ghidra_sync_structs.py --dry-run                 # list what would change
    python tool/ghidra_sync_structs.py --class Unit --class Token
    python tool/ghidra_sync_structs.py --file src/unit.h
    python tool/ghidra_sync_structs.py                           # every class in src/*.h

Merge policy (see ghidra_ops.op_struct_apply):
  - Ghidra structs are created if missing, grown to sizeof(); shrunk only if the header has ASSERT_SIZE.
  - A header field replaces undefined bytes, or a Ghidra field occupying exactly the same slot.
  - Placeholder header names (fieldN_0xXX, unk*, pad*) never overwrite a real Ghidra name/type.
  - Real Ghidra names inside the range now covered by `_` are copied into the base struct first.
  - Types unknown to Ghidra fall back to the existing component type of the same size, else undefined.
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
    Returns ({cls: {"size": n, "fields": {fname: (off, size)}}}, layout) where layout is the
    parsed /d1reportAllClassLayout output (see parse_layout_report).
    Headers that fail to compile standalone are dropped (reported) and the compile retried."""
    os.makedirs(BUILD, exist_ok=True)
    cpp = os.path.join(BUILD, "struct_dump.cpp")
    exe = os.path.join(BUILD, "struct_dump.exe")
    headers = list(headers)
    for attempt in range(12):
        _write_dump_source(cpp, selected, classes, headers)
        cmd = (f'"{VSDEVCMD}" -arch=x86 -no_logo && cl /nologo /std:c++17 /EHsc /W0 /MD /I "{SRC}" /I "{SRC}\\mfc\\include" '
               + " ".join(f"/D{d}" for d in DEFINES)
               + f' /d1reportAllClassLayout /Fo"{BUILD}\\struct_dump.obj" /Fe"{exe}" "{cpp}"')
        # cmd.exe strips the outer quotes of a /c command that starts with a quote, so wrap it once more
        r = subprocess.run(f'cmd /c "{cmd}"', capture_output=True, text=True, cwd=BUILD, encoding="utf-8", errors="replace")
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
    layout = parse_layout_report(r.stdout)
    out = subprocess.run([exe], capture_output=True, text=True).stdout
    dump = {}
    for line in out.splitlines():
        p = line.split()
        if p[0] == "CLASS":
            dump.setdefault(p[1], {"size": 0, "fields": {}})["size"] = int(p[2])
        elif p[0] == "FIELD":
            dump.setdefault(p[1], {"size": 0, "fields": {}})["fields"][p[2]] = (int(p[3]), int(p[4]))
    return dump, layout


def parse_layout_report(text):
    """Parse MSVC's /d1reportAllClassLayout output.
    Returns {cls: {"bases": [(base, offset)], "vfptr": offset|None, "vtable": [(owner, method)]}}
    where vtable lists the slots of `cls`'s primary vftable in order."""
    out = {}
    cur = None
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        m = re.match(r"^class ([A-Za-z_]\w*)\s+size\((\d+)\):", line)
        if m:
            cur = out.setdefault(m.group(1), {"bases": [], "vfptr": None, "vtable": None, "size": int(m.group(2))})
            i += 1
            # layout block: only depth-1 entries belong to this class
            while i < len(lines) and lines[i].strip() and not re.match(r"^class ", lines[i]):
                l = lines[i]
                bm = re.match(r"^\s*(\d+)\s*\|\s*\+--- \(base class ([A-Za-z_]\w*)\)", l)
                if bm:
                    cur["bases"].append((bm.group(2), int(bm.group(1))))
                vm = re.match(r"^\s*(\d+)\s*(?:\|\s*)+\{vfptr\}", l)
                if vm and cur["vfptr"] is None:
                    cur["vfptr"] = int(vm.group(1))
                i += 1
            continue
        m = re.match(r"^([A-Za-z_]\w*)::\$vftable@(\w*):", line)
        if m:
            cls, tag = m.group(1), m.group(2)
            slots = []
            i += 1
            while i < len(lines) and lines[i].strip():
                sm = re.match(r"^\s*(\d+)\s*\|\s*&(.+?)\s*$", lines[i])
                if sm:
                    target = sm.group(2)
                    om = re.match(r"([A-Za-z_]\w*)::(.+)", target)
                    slots.append((om.group(1), om.group(2).strip()) if om else ("", target))
                i += 1
            if cls in out and (not tag or out[cls]["vtable"] is None):
                out[cls]["vtable"] = slots
            continue
        i += 1
    return out


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


SPECIAL_SLOTS = {"{dtor}": ("dtor", "void", []), "GetRuntimeClass": ("GetRuntimeClass", "CRuntimeClass *", []),
                 "AssertValid": ("AssertValid", "void", []), "Dump": ("Dump", "void", ["CDumpContext * dc"])}
# CObject slots whose owner may be an MFC class we don't parse (CWordArray = CArray<> instantiation)
FALLBACK_SLOTS = {"Serialize": ("Serialize", "void", ["CArchive * ar"])}
VTABLE_HUB = "CObject"  # everything derives from it; hierarchy roots are the classes right below it


def slot_sig(owner, method, cls, decls):
    """(name, ret, params) for one vftable entry."""
    if method in SPECIAL_SLOTS:
        return SPECIAL_SLOTS[method]
    if method.startswith("_purecall") or owner == "":
        return None
    d = decls.get((owner, method)) or decls.get((cls, method))
    if d is None:
        return FALLBACK_SLOTS.get(method, (method, "void", []))
    return (method, d["ret"], [f"{t} {n}" if n else t for t, n in d["params"]])


def vtable_slots(cls, layout, decls):
    """[{name, ret, params, this}] for cls's own vftable."""
    slots = []
    for idx, (owner, method) in enumerate(layout[cls]["vtable"] or []):
        sig = slot_sig(owner, method, cls, decls)
        if sig is None:
            slots.append({"name": f"pure_virtual_{idx}", "ret": "void", "params": [], "this": cls})
        else:
            slots.append({"name": sig[0], "ret": sig[1], "params": sig[2], "this": cls})
    return slots


def chain(n, layout):
    out = [n]
    while layout.get(out[-1], {}).get("bases"):
        out.append(layout[out[-1]]["bases"][0][0])
    return out


def vtable_root(n, layout, known):
    """Top-most polymorphic ancestor of n below the hub (CObject); n itself if none."""
    root = n
    for parent in chain(n, layout)[1:]:
        if parent == VTABLE_HUB or parent not in known or not layout.get(parent, {}).get("vtable"):
            break
        root = parent
    return root


def merged_vtable_slots(root, members, layout, decls):
    """One vtable for a whole hierarchy: every descendant's slots, union where siblings disagree.
    `this` of each entry is the common ancestor of all classes defining that method (root if the
    same name is used by unrelated branches)."""
    depth = {m: len(chain(m, layout)) for m in members}
    per_slot = []
    owners = []
    for m in sorted(members, key=lambda x: depth[x]):
        for i, (owner, method) in enumerate(layout[m]["vtable"]):
            while len(per_slot) <= i:
                per_slot.append({})
                owners.append({})
            sig = slot_sig(owner, method, m, decls)
            key = sig[0] if sig else f"pure_virtual_{i}"
            owners[i].setdefault(key, set()).add(owner if owner in members else m)
            if key not in per_slot[i]:
                if sig is None:
                    per_slot[i][key] = {"name": key, "ret": "void", "params": [], "this": m}
                else:
                    per_slot[i][key] = {"name": sig[0], "ret": sig[1], "params": sig[2], "this": m}
    for i, cands in enumerate(per_slot):
        for key, c in cands.items():
            os_ = owners[i][key]
            common = [o for o in os_ if all(o in chain(x, layout) for x in os_)]
            c["this"] = min(common, key=lambda o: depth.get(o, 99)) if common else root
    out = []
    for cands in per_slot:
        vals = list(cands.values())
        out.append(vals[0] if len(vals) == 1 else vals)
    return out


def topo_order(names, layout):
    """Bases before derived classes (only among `names`)."""
    out, seen = [], set()

    def visit(n):
        if n in seen or n not in names:
            return
        seen.add(n)
        for b, _ in layout.get(n, {}).get("bases", []):
            visit(b)
        out.append(n)
    for n in names:
        visit(n)
    return out


def main():
    global PORT
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--class", dest="cls", action="append", help="only these classes (repeatable)")
    ap.add_argument("--file", help="only classes declared in this header")
    ap.add_argument("--flat", action="store_true",
                    help="flatten inherited fields into each struct and give every class its own vtable "
                         "instead of `Base _` members + one merged vtable per hierarchy root")
    ap.add_argument("--port", type=int, default=PORT)
    a = ap.parse_args()
    a.nest = not a.flat
    PORT = a.port

    headers = sorted(h for h in glob.glob(os.path.join(SRC, "*.h")) if os.path.basename(h) not in EXCLUDE_HEADERS)
    classes = parse_classes(headers)
    # keep classes that carry data or a vtable (compile errors would stop everything otherwise)
    classes = {n: c for n, c in classes.items() if all_fields(n, classes) or c["polymorphic"]}
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

    dump, layout = compile_dump(selected, classes, headers)
    decls = {}
    for d in bp.parse_headers(headers):
        if d["virtual"]:
            decls.setdefault((d["cls"], d["name"]), d)

    struct_ops, vtable_ops, size_ops, drop_ops = [], [], [], []
    poly_all = {n for n in classes if layout.get(n, {}).get("vtable")}
    if a.nest:
        # one merged vtable per hierarchy root; descendants reach it through `Base _`
        groups = {}
        for n in poly_all:
            groups.setdefault(vtable_root(n, layout, poly_all), set()).add(n)
    for name in topo_order(list(selected), layout):
        info = selected[name]
        d, lay = dump.get(name), layout.get(name)
        if not d or not lay:
            print(f"  {name}: not in dump (compile skipped?)")
            continue
        bases = lay["bases"]
        if len(bases) > 1:
            print(f"  {name}: multiple bases {bases}; only {bases[0][0]} is modelled as `_`")
        base, base_off = (bases[0] if bases else (None, 0))
        polymorphic = lay["vfptr"] is not None and bool(lay["vtable"])
        root = vtable_root(name, layout, poly_all) if polymorphic else None
        # own vtable struct: every polymorphic class (flat model) or only hierarchy roots (nest model)
        own_vtable = polymorphic and (not a.nest or root == name)
        fields = []
        own = info["fields"] if a.nest else all_fields(name, classes)
        for t, f, dims, ln in own:
            if f not in d["fields"]:
                continue
            off, size = d["fields"][f]
            fields.append({"offset": off, "size": size, "type": t + dims, "name": f,
                           "placeholder": bool(PLACEHOLDER_FIELD.match(f))})
        args = {"name": name, "size": d["size"], "fields": fields, "shrink": info["assert_size"] is not None,
                "polymorphic": polymorphic, "base": base if a.nest else None, "base_offset": base_off,
                "vtable": own_vtable}
        size_ops.append({"op": "struct-apply", "args": {**args, "fields": None}})
        if own_vtable:
            slots = merged_vtable_slots(name, groups[name], layout, decls) if a.nest else vtable_slots(name, layout, decls)
            vtable_ops.append({"op": "vtable-apply", "args": {"cls": name, "slots": slots}})
        elif polymorphic and a.nest:
            drop_ops.append({"op": "vtable-remove", "args": {"cls": name}})
        struct_ops.append({"op": "struct-apply", "args": args})
        if a.dry_run:
            vt = f"  vtable={len(slots)} slots" if own_vtable else (f"  vtable: via {root}_VTable" if polymorphic else "")
            bs = f"  base={base}@0x{base_off:X}" if base else ""
            print(f"{name}  sizeof=0x{d['size']:X}{bs}{vt}  assert_size={info['assert_size']}  [{info['file']}:{info['line']}]")
            if own_vtable:
                for i, sl in enumerate(slots):
                    if isinstance(sl, list):
                        print(f"    [{i:2}] union: " + " | ".join(f"{c['this']}::{c['name']}" for c in sl))
                    else:
                        print(f"    [{i:2}] {sl['ret']} {sl['name']}({', '.join(sl['params'])})")
            for fl in fields:
                print(f"    0x{fl['offset']:04X} {fl['size']:4}  {fl['type']:28} {fl['name']}")
    if a.dry_run:
        return
    # pass 1: make every struct exist with its size (vtable `this` params and by-value members need them)
    for line in call("batch", {"ops": size_ops}).splitlines():
        if line.startswith("ERR"):
            print(line)
    # pass 2: vtables, pass 3: full layouts (bases first), pass 4: drop per-class vtables made obsolete by --nest
    for ops in (vtable_ops, struct_ops, drop_ops):
        for i in range(0, len(ops), 100):
            for line in call("batch", {"ops": ops[i:i + 100]}).splitlines():
                print(line if line.startswith("ERR") else line.split(" -> ", 1)[-1])


if __name__ == "__main__":
    main()
