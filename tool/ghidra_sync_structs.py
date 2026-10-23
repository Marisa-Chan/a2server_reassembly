#!/usr/bin/env python
"""Push class/struct layouts (and vtables) from our C++ headers into Ghidra (via the daemon).

Nothing is guessed: a generated C++ program `#include`s the headers and is compiled with the
project's MSVC x86 toolset. `offsetof`/`sizeof` give field offsets; `/d1reportAllClassLayout`
gives base-class offsets, vfptr placement and the exact vtable slot order (overrides resolved).

Resulting Ghidra model:
  - Every class struct carries ALL its data fields, inherited ones included (flattened, with the
    comment `Inherited from Base`), so `this->hp` resolves directly and indexed access into base
    arrays stays readable.
  - Every polymorphic class gets its own `{Class}_VTable` (category /Game/VTables): one typed
    __thiscall function pointer per slot in MSVC's order, signatures from the headers. Offset 0 is
    `{Class}_VTable *vptr`, so virtual calls render as `(*this->vptr->VMethod25)(this)`.

    python tool/ghidra_sync_structs.py --dry-run                 # list what would change
    python tool/ghidra_sync_structs.py --class Unit --class Token
    python tool/ghidra_sync_structs.py --file src/unit.h
    python tool/ghidra_sync_structs.py                           # every class in src/*.h

Merge policy (see ghidra_ops.op_struct_apply):
  - Ghidra structs are created if missing, grown to sizeof(); shrunk only if the header has ASSERT_SIZE.
  - A header field replaces undefined bytes, or a Ghidra field occupying exactly the same slot.
  - Placeholder header names (fieldN_0xXX, unk*, pad*) never overwrite a real Ghidra name/type.
  - Existing Ghidra field comments are kept; `Inherited from` is only added where there is none.
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
PROGRAM = None  # explicit program selection ("server"/"client"/...); None = daemon default
VSDEVCMD = r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
DEFINES = ["WIN32", "_WINDOWS", "FKG_FORCED_USAGE", "_USE_32BIT_TIME_T", "NDEBUG", "_CONSOLE",
           "_CRT_SECURE_NO_WARNINGS", "A2SERVER_PATCH", "A2SERVER_TANGAR_HAT", "_AFXDLL"]
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

# MFC types whose size differs between the game's VC6 MFC and the MFC we compile
# against. CObject-rooted containers are identical (CObject/CString=4, CArray/
# CWordArray/CDWordArray/CStringArray/CMap=0x1c/0x14), but the CWnd family, CFile,
# CTime (time_t 4->8) and CWinThread differ, so offsets compiled from our headers
# are WRONG for classes that inherit them or embed them as members.
GROWN_MFC = {"CWnd", "CDialog", "CFrameWnd", "CWinApp", "CListBox", "CEdit", "CStatic",
             "CButton", "CStatusBar", "CScrollBar", "CComboBox", "CSpinButtonCtrl",
             "CSliderCtrl", "CProgressCtrl", "CFile", "CFileFind", "CStdioFile",
             "CMemFile", "COleDataObject", "CWinThread", "CTime"}

# Hand-verified retail-binary layouts for the affected classes (offsets from ctor
# disassembly / original binary asserts). "__size__" gives the binary class size.
# Classes affected but NOT listed here are skipped by the sync with a warning -
# never push offsets known to disagree with the binary.
BIN_LAYOUT = {
    # both programs share these; MainWindow differs per build (client class is
    # 0x800: extra dword at 0x3d0, +4 shift through field_0x460, no cursor handles,
    # no field_0x464/0x5e4/0x804, CGameSession 0x190)
    "File2": {"__size__": 0x20, "file": 0x10, "offset": 0x14, "size": 0x18, "position": 0x1c},
    "PasswordManager": {"__size__": 0x1c, "content": 0x0, "fname": 0x14, "date": 0x18},
    "ServerControlDialog": {"__size__": 0x280, "edit_5c": 0x5c, "static_98": 0x98,
                            "button_d4": 0xd4, "button_110": 0x110, "button_14c": 0x14c,
                            "button_188": 0x188, "button_1c4": 0x1c4, "button_200": 0x200,
                            "button_23c": 0x23c, "str_278": 0x278, "str_27c": 0x27c},
    "VisNetMapSelection": {"__size__": 0xfc, "p_mapname": 0x68, "avail_maps": 0x6c,
                           "thread": 0x80, "stop_event": 0xe8, "selected_map_index": 0xec,
                           "chat_log": 0xf0, "map_context": 0xf4, "mode": 0xf8},
}
_MW_COMMON = {
    "has_focus": 0xbc, "console_active": 0xc0, "music_update_proc": 0xc4, "music_player": 0xc8,
    "vis_root": 0xcc, "vis_map_context": 0xd0, "vis_right_panel": 0xd4, "vis_minimap": 0xd8,
    "vis_ordertoolbar": 0xdc, "vis_charinfo": 0xe0, "vis_sidestatus": 0xe4, "vis_invtype1": 0xe8,
    "vis_spellbook": 0xec, "vis_globalmap": 0xf0, "vis_mainmenu": 0xf4, "vis_shop": 0xf8,
    "vis_shopdruid": 0xfc, "vis_shopkaarg": 0x100, "vis_tav": 0x104, "vis_tavdruid": 0x108,
    "vis_tavkaarg": 0x10c, "vis_town": 0x110, "vis_towndruid": 0x114, "vis_townkaarg": 0x118,
    "vis_famehall": 0x11c, "vis_dropgold": 0x120, "vis_missionfailed": 0x124, "vis_2action": 0x128,
    "field_0x12c": 0x12c, "vis_credits": 0x130, "vis_logownd": 0x134, "vis_1200": 0x138,
    "vis_loadgame": 0x13c, "vis_savegame": 0x140, "vis_msgbox": 0x144, "last_save_info": 0x148,
    "diplomacy_entries": 0x348, "field_0x34c": 0x34c, "vis_connectiondlg": 0x350,
    "net_sessions": 0x354, "last_protocol": 0x368, "vis_chargen": 0x36c, "vis_charsel": 0x370,
    "vis_startgame": 0x374, "vis_netmapsel": 0x378, "vis_hatbrowser": 0x37c,
    "phone_book": 0x380, "com_settings": 0x39c, "last_ip": 0x3b0, "hat_settings": 0x3b4,
}
BIN_LAYOUT["MainWindow"] = {
    "server": {"__size__": 0x808, **_MW_COMMON,
               "vis_serverscreen": 0x3d0, "vis_cutscenes": 0x3d4, "vis_famedocument": 0x3d8,
               "vis_msgbox_list": 0x3dc, "field_0x3e0": 0x3e0, "in_famehall_mission": 0x3f8,
               "mission_failed_reported": 0x3fc, "field_0x400": 0x400, "pending_game_mode": 0x404,
               "drag_entry": 0x408, "drag_src_slot": 0x40c, "drag_src_tab": 0x410,
               "item_cursor": 0x414, "dialogsMask": 0x418, "serverLoopCounter": 0x41c,
               "game_tic_counter": 0x420, "field_0x424": 0x424, "last_tic_time": 0x428,
               "game_tic_time": 0x42c, "game_speed": 0x430, "field_0x434": 0x434,
               "field_0x438": 0x438, "field_0x43c": 0x43c, "field_0x440": 0x440,
               "field_0x444": 0x444, "unlimited_speed": 0x448, "field_0x44c": 0x44c,
               "last_event_id": 0x450, "inside_building": 0x454, "enter_ack_received": 0x458,
               "input_locked": 0x45c, "field_0x460": 0x460, "field_0x464": 0x464,
               "m_GameSession": 0x468, "field_0x5e4": 0x5e4, "m_FameHall": 0x5e8,
               "current_map_name": 0x620, "shutdown_pending": 0x624, "cursor_sizewe": 0x628,
               "cursor_sizens": 0x62c, "cursor_sizenwse": 0x630, "cursor_sizenesw": 0x634,
               "cursor_arrow": 0x638, "serverBootstrapEnabled": 0x63c, "sessionMode": 0x640,
               "clip_cursor_rect": 0x644, "status_bar": 0x654, "list_box1": 0x6d0,
               "list_box2": 0x70c, "static1": 0x748, "static2": 0x784, "edit": 0x7c0,
               "server_ctrl_dlg": 0x7fc, "server_ctrl_shown": 0x800, "field_0x804": 0x804},
    "client": {"__size__": 0x800, **_MW_COMMON,
               "field_0x3d0_cli": 0x3d0, "vis_serverscreen": 0x3d4, "vis_cutscenes": 0x3d8,
               "vis_famedocument": 0x3dc, "vis_msgbox_list": 0x3e0, "field_0x3e0": 0x3e4,
               "in_famehall_mission": 0x3fc, "mission_failed_reported": 0x400,
               "field_0x400": 0x404, "pending_game_mode": 0x408, "drag_entry": 0x40c,
               "drag_src_slot": 0x410, "drag_src_tab": 0x414, "item_cursor": 0x418,
               "dialogsMask": 0x41c, "serverLoopCounter": 0x420, "game_tic_counter": 0x424,
               "field_0x424": 0x428, "last_tic_time": 0x42c, "game_tic_time": 0x430,
               "game_speed": 0x434, "field_0x434": 0x438, "field_0x438": 0x43c,
               "field_0x43c": 0x440, "field_0x440": 0x444, "field_0x444": 0x448,
               "unlimited_speed": 0x44c, "field_0x44c": 0x450, "last_event_id": 0x454,
               "inside_building": 0x458, "enter_ack_received": 0x45c, "input_locked": 0x460,
               "field_0x460": 0x464, "m_GameSession": 0x468, "m_FameHall": 0x5f8,
               "current_map_name": 0x630, "shutdown_pending": 0x634,
               "serverBootstrapEnabled": 0x638, "sessionMode": 0x63c, "clip_cursor_rect": 0x640,
               "status_bar": 0x650, "list_box1": 0x6cc, "list_box2": 0x708, "static1": 0x744,
               "static2": 0x780, "edit": 0x7bc, "server_ctrl_dlg": 0x7f8,
               "server_ctrl_shown": 0x7fc},
}


def _grown_mfc_affected(name, classes, seen=None):
    """True if `name` inherits from or embeds a member of a size-changed MFC type."""
    seen = seen or set()
    if name in seen or name not in classes:
        return name in GROWN_MFC
    seen.add(name)
    info = classes[name]
    for b in info["bases"]:
        base = b.split()[-1].strip("*")
        if base in GROWN_MFC or _grown_mfc_affected(base, classes, seen):
            return True
    for fld in info["fields"]:
        base = fld[0].strip().strip("*").split("<")[0]
        if base in GROWN_MFC:
            return True
    return False


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
    """Own + inherited fields (base first) as (type, name, dims, line, owner_class); unknown bases skipped."""
    seen = seen or set()
    if name in seen or name not in classes:
        return []
    seen.add(name)
    out = []
    for b in classes[name]["bases"]:
        out.extend(all_fields(b, classes, seen))
    out.extend((t, f, dims, ln, name) for t, f, dims, ln in classes[name]["fields"])
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
    # Per-build guards (#ifdef A2CLIENT / #ifndef A2CLIENT) mean a parsed field may be
    # compiled out for the selected variant, so every probe is SFINAE-guarded: an
    # absent member prints nothing instead of failing the whole dump compile.
    blocks = []  # (class, [(probe_id, field), ...])
    idx = 0
    for name, info in selected.items():
        fields = []
        for t, f, dims, ln, owner in all_fields(name, classes):
            idx += 1
            fields.append((idx, f))
        blocks.append((name, fields))
    with open(cpp, "w", encoding="utf-8") as fh:
        fh.write("#define _ALLOW_KEYWORD_MACROS\n#define private public\n#define protected public\n#include <cstddef>\n#include <cstdio>\n#include <type_traits>\n")
        fh.write("#pragma warning(disable: 4267 4244 4996)\n")
        for h in headers:
            fh.write(f'#include "{os.path.relpath(h, BUILD).replace(os.sep, "/")}"\n')
        # classes declared inside per-build #ifdef blocks may not exist in this compile
        # variant; forward-declare everything so the name exists, and guard each probe
        # with completeness/member detection so absent ones print nothing
        for name, fields in blocks:
            fh.write(f"class {name};\n")
        fh.write("template <class T, class = void> struct Complete : std::false_type {};\n"
                 "template <class T> struct Complete<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};\n")
        for name, fields in blocks:
            fh.write(f"template <class C> static void PC_{name}() {{\n"
                     f"  if constexpr (Complete<C>::value) {{\n"
                     f'    printf("CLASS {name} %u\\n", (unsigned)sizeof(C));\n'
                     f"  }}\n"
                     f"}}\n")
            for i, f in fields:
                fh.write(f"template <class T, class = void> struct Has_f{i} : std::false_type {{}};\n"
                         f"template <class T> struct Has_f{i}<T, std::void_t<decltype(&T::{f})>> : std::true_type {{}};\n"
                         f"template <class C> static void P_f{i}() {{\n"
                         f"  if constexpr (Has_f{i}<C>::value) {{\n"
                         f'    printf("FIELD {name} {f} %u %u\\n", (unsigned)offsetof(C, {f}), (unsigned)sizeof(((C*)0)->{f}));\n'
                         f"  }}\n"
                         f"}}\n")
        fh.write("int main() {\n")
        for name, fields in blocks:
            fh.write(f"  PC_{name}<{name}>();\n")
            for i, f in fields:
                fh.write(f"  P_f{i}<{name}>();\n")
        fh.write("  return 0;\n}\n")


def call(op, args):
    payload = {"op": op, "args": args}
    if PROGRAM:
        payload["program"] = PROGRAM
    data = json.dumps(payload).encode("utf-8")
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
    """[{name, ret, params}] for cls's vftable."""
    slots = []
    for idx, (owner, method) in enumerate(layout[cls]["vtable"] or []):
        sig = slot_sig(owner, method, cls, decls)
        if sig is None:
            slots.append({"name": f"pure_virtual_{idx}", "ret": "void", "params": []})
        else:
            slots.append({"name": sig[0], "ret": sig[1], "params": sig[2]})
    return slots


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
    global PORT, PROGRAM
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--class", dest="cls", action="append", help="only these classes (repeatable)")
    ap.add_argument("--file", help="only classes declared in this header")
    ap.add_argument("--port", type=int, default=PORT)
    sel = ap.add_mutually_exclusive_group()
    sel.add_argument("--client", action="store_true", help="apply to the client program (allods2.exe)")
    sel.add_argument("--server", action="store_true", help="apply to the server program (a2serv6.exe_new, default)")
    sel.add_argument("--program", help="program by name (server/client/...) - deliberately no env var: "
                                       "bulk pushes should be explicit")
    a = ap.parse_args()
    PORT = a.port
    PROGRAM = "client" if a.client else "server" if a.server else a.program
    if PROGRAM is None:
        PROGRAM = "server"  # daemon default; also keys the BIN_LAYOUT per-program tables
    if PROGRAM == "client":
        # compile the per-build header variant: #ifdef A2CLIENT blocks (MainWindow,
        # CGameSession, ...) carry the client layout, and server-only patches stay out
        DEFINES[:] = [d for d in DEFINES if d != "A2SERVER_PATCH"] + ["A2CLIENT"]

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

    struct_ops, vtable_ops, size_ops = [], [], []
    for name in topo_order(list(selected), layout):
        info = selected[name]
        d, lay = dump.get(name), layout.get(name)
        if not d or not lay:
            print(f"  {name}: not in dump (compile skipped?)")
            continue
        override = BIN_LAYOUT.get(name)
        if isinstance(override, dict) and ("server" in override or "client" in override):
            override = override.get(PROGRAM)
        if override is None and _grown_mfc_affected(name, classes):
            print(f"  {name}: SKIPPED - MFC-size-affected class without a verified BIN_LAYOUT table")
            continue
        if override is not None:
            offs = sorted((o, f) for f, o in override.items() if f != "__size__")
            flds = {}
            for k, (o, f) in enumerate(offs):
                end = offs[k + 1][0] if k + 1 < len(offs) else override["__size__"]
                flds[f] = (o, end - o)
            d = {"size": override["__size__"], "fields": flds}
        polymorphic = lay["vfptr"] is not None and bool(lay["vtable"])
        fields = []
        for t, f, dims, ln, owner in all_fields(name, classes):
            if f not in d["fields"]:
                continue
            off, size = d["fields"][f]
            fields.append({"offset": off, "size": size, "type": t + dims, "name": f,
                           "placeholder": bool(PLACEHOLDER_FIELD.match(f)),
                           "comment": None if owner == name else f"Inherited from {owner}"})
        args = {"name": name, "size": d["size"], "fields": fields,
                "shrink": info["assert_size"] is not None or override is not None,
                "remove_beyond": override is not None,
                "vtable": polymorphic}
        size_ops.append({"op": "struct-apply", "args": {**args, "fields": None}})
        slots = vtable_slots(name, layout, decls) if polymorphic else []
        if polymorphic:
            vtable_ops.append({"op": "vtable-apply", "args": {"cls": name, "slots": slots}})
        struct_ops.append({"op": "struct-apply", "args": args})
        if a.dry_run:
            vt = f"  vtable={len(slots)} slots" if polymorphic else ""
            print(f"{name}  sizeof=0x{d['size']:X}{vt}  assert_size={info['assert_size']}  [{info['file']}:{info['line']}]")
            for i, sl in enumerate(slots):
                print(f"    [{i:2}] {sl['ret']} {sl['name']}({', '.join(sl['params'])})")
            for fl in fields:
                cm = f"  // {fl['comment']}" if fl["comment"] else ""
                print(f"    0x{fl['offset']:04X} {fl['size']:4}  {fl['type']:28} {fl['name']}{cm}")
    if a.dry_run:
        return
    # pass 1: make every struct exist with its size (vtable `this` params and by-value members need them)
    for line in call("batch", {"ops": size_ops}).splitlines():
        if line.startswith("ERR"):
            print(line)
    # pass 2: vtables, pass 3: full layouts (bases first)
    for ops in (vtable_ops, struct_ops):
        for i in range(0, len(ops), 100):
            for line in call("batch", {"ops": ops[i:i + 100]}).splitlines():
                print(line if line.startswith("ERR") else line.split(" -> ", 1)[-1])


if __name__ == "__main__":
    main()
