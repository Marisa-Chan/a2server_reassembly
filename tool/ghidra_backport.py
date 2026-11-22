#!/usr/bin/env python
"""Push function prototypes from our C++ headers/sources into Ghidra (via the daemon)
and verify VA comment consistency against both retail binaries.

The headers and .cpp files are the source of truth: every retail function whose
address is known - from its standardized dual-VA comment (`// <srv> (client: <cli>)`,
`// <srv> (client: none)`, `// server: none (client: <cli>)`) or legacy VA marker -
is applied to Ghidra as name + class + calling convention + return/param types + names.

Split helpers (`split from ...`), inlined snippets (`inlined in ...`), and
Scenario.dll entries (`fnptr: ...` / `scenario.dll: ...`) are excluded from normal
.text prototype pushes, while `--verify` validates their comment format and parity.

    python tool/ghidra_backport.py --dry-run                 # list what would be applied (server)
    python tool/ghidra_backport.py --client --dry-run        # list what would be applied (client)
    python tool/ghidra_backport.py --verify                  # verify server VAs, parity & Ghidra state
    python tool/ghidra_backport.py --client --verify         # verify client VAs, parity & Ghidra state
    python tool/ghidra_backport.py --class QuestMap          # one class
    python tool/ghidra_backport.py --addr 55ECFE             # one function
    python tool/ghidra_backport.py --file src/quest_map.h    # one header
    python tool/ghidra_backport.py                           # everything (server)
    python tool/ghidra_backport.py --client                  # everything (client)
"""
import argparse
from collections import defaultdict
import glob
import json
import os
import re
import sys
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
PORT = 18812
PROGRAM = None  # explicit program selection ("server"/"client"/...); None = daemon default

EXCLUDED_BASENAMES = {
    "mfc_templ_impl.h", "dplay.h", "dplobby.h", "2darray.h", "bitset.h",
    "asm_mfc.cpp", "a2_ucrt.cpp", "cmain.cpp", "decomp.c",
}

SPECIAL_PP_SYMS = ("A2CLIENT", "A2SERVER_PATCH", "A2CLIENT_PATCH", "A2TANGAR_HAT")

# Intentional C++ non-retail declarations/definitions (no retail binary VA).
# Param signature may be "*" to skip all overloads of (cls, name).
NON_RETAIL_SKIPS = {
    ("None", "GetParsedPlayerStats", "*"),
    ("None", "GetParsedPlayerBasicInfo", "*"),
    ("None", "PacketType", "*"),
    ("None", "CheckPacketType", "*"),
    ("DiplomacyEntry", "DiplomacyEntry", ()),
    ("Unit", "EffectiveScanRange", "*"),
    ("CGamePalette", "operator=", "*"),
    ("PacketWord", "PacketWord", ("PacketWord *",)),
    ("PacketGroupOrder", "PacketGroupOrder", ("PacketGroupOrder *",)),
    ("PacketItemTransfer", "PacketItemTransfer", ("PacketItemTransfer *",)),
    ("PacketEnterGame", "PacketEnterGame", ("PacketEnterGame *",)),
    ("TakeDamage", "TakeDamage", ("TakeDamage *",)),
    ("PacketUnitEquipment", "ReadFrom", "*"),
    ("SmackerPlayer", "CreateAudioBuffer", "*"),
    ("SmackerPlayer", "WriteAudioChunk", "*"),
    ("World", "GetAutobuffSpell", "*"),
}


def _is_non_retail(cls, name, ptypes):
    c = cls or "None"
    lpt = tuple(_norm_t_loose(p) for p in ptypes)
    if (c, name, "*") in NON_RETAIL_SKIPS:
        return True
    for vc, vn, vpt in NON_RETAIL_SKIPS:
        if vc == c and vn == name and vpt != "*" and tuple(_norm_t_loose(p) for p in vpt) == lpt:
            return True
    return False

# Verified MSVC /OPT:ICF folds (identical retail machine code folded into one address).
ICF_ALLOWLIST = {
    "server": {},
    "client": {
        "547970": frozenset(["Group::GetNextUnit", "PlayersList::GetNext"]),
    },
}

CANONICAL_COMMENT_RE = re.compile(
    r"^("
    r"[0-9a-f]{6} \(client: (?:none|[0-9a-f]{6})\)"
    r"|server: none \(client: [0-9a-f]{6}\)"
    r"|(?:split from|inlined in) (?:[0-9a-f]{6}|server: none) \(client: (?:none|[0-9a-f]{6})\)"
    r"|fnptr: [0-9a-f]{6} \(client: [0-9a-f]{6}\), scenario\.dll: [0-9a-f]{8}"
    r"|(?:split from )?scenario\.dll: [0-9a-f]{8}"
    r")(?:\s+(?:-|—)\s+.+)?$"
)

ADDR_IN_NAME = re.compile(r"^(?:sub_([0-9A-Fa-f]{6})|FUN_00([0-9A-Fa-f]{6}))$")
ADDR_IN_COMMENT = re.compile(r"(?<![\w])(?:sub_|FUN_00|0x)?((?=[0-9a-fA-F]*\d)[0-9A-Fa-f]{6})(?![\w])")
CLASS_RE = re.compile(r"^\s*(class|struct|namespace)\s+([A-Za-z_]\w*)\s*(?::[^{;]*)?\{")
CLASS_OPEN_RE = re.compile(r"^\s*(class|struct|namespace)\s+([A-Za-z_]\w*)\s*(?::[^{;]*)?$")
DECL_HEAD_RE = re.compile(
    r"^\s*(?P<kw>(?:(?:virtual|static|inline|explicit|constexpr)\s+)*)"
    r"(?P<ret>(?:[A-Za-z_][\w:]*(?:<[^;{}()]*?>)?[\s\*&]+(?:const\s*[\*&]*\s*)?)*?)"
    r"(?P<name>~?[A-Za-z_]\w*|operator\s*(?:\(\)|[+\-*/%^&|~!<>=,\[\]]+|[A-Za-z_]\w*(?:\s*[\*&]+)?))\s*\("
)
SKIP_NAMES = {"if", "for", "while", "switch", "return", "sizeof", "ASSERT_OFFSET", "ASSERT_SIZE", "static_assert"}


def _is_excluded(path):
    rel = os.path.relpath(path, ROOT).replace("\\", "/")
    base = os.path.basename(path)
    return base in EXCLUDED_BASENAMES or rel.startswith("src/patch/")


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


PROTO_TYPE_ALIASES = {
    "AssortFlags": "uint32_t",
    "QuestId": "uint32_t",
    "QuestLandmarkId": "uint32_t",
    "LPCDPNAME": "void *",
    "LPCDPSESSIONDESC2": "void *",
    "CBUPDATECALLBACK": "void *",
    "DPID": "uint32_t",
    "REFGUID": "void *",
    "LPCGUID": "void *",
}


def _proto_type(t):
    for k, v in PROTO_TYPE_ALIASES.items():
        if t == k:
            return v
        if t.startswith(k + " "):
            return (v + t[len(k):]).replace("* *", "**")
    return t


def norm_type(t):
    if "(" in t:  # function pointer
        return "void *"
    t = re.sub(
        r"\b(const|class|struct|enum|constexpr|inline|static|virtual|explicit|"
        r"AFX_CDECL|AFXAPI|PASCAL|WINAPI|CALLBACK|__cdecl|__stdcall|__fastcall|__thiscall)\b",
        " ",
        t,
    )
    t = t.replace("&", "*")
    t = re.sub(r"\s+", " ", t).strip()
    t = re.sub(r"\s*\*\s*", " *", t).strip()
    return t


def parse_param(p):
    p = re.sub(r"/\*.*?\*/", " ", p)
    p = p.split("=")[0].strip()
    if p in ("void", "..."):
        return None
    m = re.fullmatch(r"(.+?[\s\*&>])\s*([A-Za-z_]\w*)\s*(\[[^\]]*\])?", p)
    if m and not re.fullmatch(r"(unsigned|signed|const|char|short|int|long)", m.group(2)) and \
            not m.group(1).strip().endswith(("<", ",")):
        t = norm_type(m.group(1)) + (" *" if m.group(3) else "")
        return t, m.group(2)
    return norm_type(p), None


def param_types(params):
    return tuple(t for t, _ in params)


def _strip_line_comments_and_strings(s):
    out = []
    i, n = 0, len(s)
    in_str = None
    while i < n:
        ch = s[i]
        if in_str:
            if ch == "\\" and i + 1 < n:
                i += 2
                continue
            if ch == in_str:
                in_str = None
            i += 1
            continue
        if ch == "/" and i + 1 < n and s[i + 1] == "/":
            break
        if ch in ('"', "'"):
            in_str = ch
            i += 1
            continue
        out.append(ch)
        i += 1
    return "".join(out)


def _extract_balanced_decl(text, open_paren_pos):
    """Given text starting before or at '(' after name, return (params_str, rest_after_paren) or (None, None)."""
    i = open_paren_pos
    n = len(text)
    depth = 1
    params = []
    while i < n and depth > 0:
        ch = text[i]
        if ch == "(":
            depth += 1
            params.append(ch)
        elif ch == ")":
            depth -= 1
            if depth > 0:
                params.append(ch)
        else:
            params.append(ch)
        i += 1
    if depth != 0:
        return None, None
    return "".join(params), text[i:]


def _eval_cond_bits(cond):
    has_special = any(re.search(rf"\b{s}\b", cond) for s in SPECIAL_PP_SYMS)
    if not has_special:
        v = cond.strip() != "0"
        return {"srv_b": v, "cli_b": v, "srv_p": v, "cli_p": v}

    def check_target(is_cli, with_patch=True):
        c = cond
        c = re.sub(
            r"defined\s*\(\s*(\w+)\s*\)",
            lambda m: "1" if (
                (m.group(1) == "A2CLIENT" and is_cli) or
                (with_patch and m.group(1) == "A2SERVER_PATCH" and not is_cli) or
                (with_patch and m.group(1) == "A2CLIENT_PATCH" and is_cli) or
                (with_patch and m.group(1) == "A2TANGAR_HAT")
            ) else ("0" if m.group(1) in SPECIAL_PP_SYMS else "1"),
            c,
        )
        c = re.sub(r"\bA2CLIENT\b", "1" if is_cli else "0", c)
        c = re.sub(r"\bA2SERVER_PATCH\b", "1" if (with_patch and not is_cli) else "0", c)
        c = re.sub(r"\bA2CLIENT_PATCH\b", "1" if (with_patch and is_cli) else "0", c)
        c = re.sub(r"\bA2TANGAR_HAT\b", "1" if with_patch else "0", c)
        c = re.sub(r"\b[A-Za-z_]\w*\b", "1", c)
        c = c.replace("&&", " and ").replace("||", " or ").replace("!", " not ")
        try:
            return bool(eval(c, {"__builtins__": {}}, {}))
        except Exception:
            return True

    return {
        "srv_b": check_target(False, False),
        "cli_b": check_target(True, False),
        "srv_p": check_target(False, True),
        "cli_p": check_target(True, True),
    }


def _pp_push(pp_stack, kw, rest):
    if kw == "ifdef":
        b = _eval_cond_bits(f"defined({rest})")
        pp_stack.append({**b, "t_srv_b": b["srv_b"], "t_cli_b": b["cli_b"], "t_srv_p": b["srv_p"], "t_cli_p": b["cli_p"]})
    elif kw == "ifndef":
        b = _eval_cond_bits(f"!defined({rest})")
        pp_stack.append({**b, "t_srv_b": b["srv_b"], "t_cli_b": b["cli_b"], "t_srv_p": b["srv_p"], "t_cli_p": b["cli_p"]})
    elif kw == "if":
        b = _eval_cond_bits(rest)
        pp_stack.append({**b, "t_srv_b": b["srv_b"], "t_cli_b": b["cli_b"], "t_srv_p": b["srv_p"], "t_cli_p": b["cli_p"]})
    elif kw == "elif" and pp_stack:
        b = _eval_cond_bits(rest)
        f = pp_stack[-1]
        for k in ("srv_b", "cli_b", "srv_p", "cli_p"):
            f[k] = (not f["t_" + k]) and b[k]
            f["t_" + k] = f["t_" + k] or b[k]
    elif kw == "else" and pp_stack:
        f = pp_stack[-1]
        for k in ("srv_b", "cli_b", "srv_p", "cli_p"):
            f[k] = not f["t_" + k]
            f["t_" + k] = True
    elif kw == "endif" and pp_stack:
        pp_stack.pop()


def _parse_pp_flags(line):
    m = re.match(r"^\s*#\s*(ifdef|ifndef|if|elif|else|endif)\b(.*)", line)
    return (m.group(1), m.group(2).strip()) if m else None


def _eval_vis(stack):
    if not stack:
        return True, True, False
    srv_b = all(x["srv_b"] for x in stack)
    cli_b = all(x["cli_b"] for x in stack)
    srv_p = all(x["srv_p"] for x in stack)
    cli_p = all(x["cli_p"] for x in stack)
    return (srv_b or srv_p, cli_b or cli_p, (srv_p or cli_p) and not (srv_b or cli_b))


def parse_va_comment(tail):
    """Parse standardized dual-VA or legacy VA comments.
    Returns metadata dict; non-normal kinds (split, inlined, fnptr, scenario_dll)
    intentionally yield srv=None, cli=None so they are never pushed as normal protos.
    """
    cmt = tail.split("//", 1)[1].strip() if "//" in tail else ""
    base = {
        "kind": None, "srv": None, "cli": None, "srv_none": False, "cli_none": False,
        "split_srv": None, "split_cli": None, "bss_srv": None, "bss_cli": None,
        "dll_va": None, "raw": cmt,
    }
    if not cmt:
        return base

    # 1. Scenario.dll fnptr / direct scenario.dll markers
    mfn = re.match(
        r"^fnptr:\s*([0-9A-Fa-f]{6})\s+\(client:\s*([0-9A-Fa-f]{6})\),\s*scenario\.dll:\s*([0-9A-Fa-f]{6,8})\b",
        cmt,
        re.I,
    )
    if mfn:
        base["kind"] = "fnptr"
        base["bss_srv"] = mfn.group(1).upper()
        base["bss_cli"] = mfn.group(2).upper()
        base["dll_va"] = mfn.group(3).upper()
        return base

    mdll = re.match(r"^(split\s+from\s+)?scenario\.dll:\s*(?:0x)?([0-9A-Fa-f]{6,8})\b", cmt, re.I)
    if mdll:
        base["kind"] = "scenario_split" if mdll.group(1) else "scenario_dll"
        base["dll_va"] = mdll.group(2).upper()
        return base

    mdll_bare = re.match(r"^(?:0x)?(100[0-9A-Fa-f]{5})\b", cmt)
    if mdll_bare:
        base["kind"] = "scenario_dll"
        base["dll_va"] = mdll_bare.group(1).upper()
        return base

    # 2. Split / inlined markers (never push as own standalone function entry)
    mspl = re.match(
        r"^(split\s+from|inlined\s+in)\s+(?:(server:\s*none)|([0-9A-Fa-f]{6}))\s+\(client:\s*(none|([0-9A-Fa-f]{6}))\)",
        cmt,
        re.I,
    )
    if mspl:
        base["kind"] = "inlined" if mspl.group(1).lower().startswith("inlined") else "split"
        if mspl.group(2):
            base["srv_none"] = True
        elif mspl.group(3):
            base["split_srv"] = mspl.group(3).upper()
        if mspl.group(4).lower() == "none":
            base["cli_none"] = True
        else:
            base["split_cli"] = mspl.group(5).upper()
        return base

    # 3. Standardized normal dual-VA comment forms (before unanchored legacy split/inlined checks)
    mstd1 = re.match(r"^([0-9A-Fa-f]{6})\s+\(client:\s*(none|([0-9A-Fa-f]{6}))\)", cmt, re.I)
    if mstd1:
        base["kind"] = "normal"
        base["srv"] = mstd1.group(1).upper()
        if mstd1.group(2).lower() == "none":
            base["cli_none"] = True
        else:
            base["cli"] = mstd1.group(3).upper()
        return base

    mstd2 = re.match(r"^server:\s*none\s+\(client:\s*([0-9A-Fa-f]{6})\)", cmt, re.I)
    if mstd2:
        base["kind"] = "normal"
        base["srv_none"] = True
        base["cli"] = mstd2.group(1).upper()
        return base

    # Legacy split/inlined forms
    mspl_old = re.search(r"\b(?:split\s+from|from)\s+(?:0x|sub_)?([0-9A-Fa-f]{6})\b", cmt, re.I)
    minl_old = re.search(r"\binlined\s+in\s+(?:0x|sub_)?([0-9A-Fa-f]{6})\b", cmt, re.I)
    if re.search(r"\bsplit\s+from\b|\bfrom\s+[0-9A-Fa-f]{6}\b", cmt, re.I) or minl_old:
        base["kind"] = "inlined" if minl_old else "split"
        mref = minl_old or mspl_old
        base["split_srv"] = mref.group(1).upper() if mref else None
        mc = re.search(r"\bclient:\s*(none|(?:0x|sub_)?([0-9A-Fa-f]{6}))\b", cmt, re.I)
        if mc:
            if mc.group(1).lower() == "none":
                base["cli_none"] = True
            else:
                base["split_cli"] = mc.group(2).upper()
        return base

    # 4. Fallback for pre-standardization comments
    cli, cli_none = None, bool(re.search(r"\bclient:\s*none\b", cmt, re.I))
    if not cli_none:
        km = re.search(r"\bclient:\s*(?:0x|sub_)?([0-9A-Fa-f]{6})\b", cmt, re.I)
        if km:
            cli = km.group(1).upper()
        else:
            km2 = re.search(r"\bAllods2\.exe\s+(?:0x|sub_)?([0-9A-Fa-f]{6})\b", cmt, re.I)
            if km2:
                cli = km2.group(1).upper()
    srv, srv_none = None, bool(re.search(r"\bserver:\s*none\b", cmt, re.I))
    if not srv_none:
        t2 = re.sub(r"\(?\bclient:\s*(?:none|(?:0x|sub_)?[0-9A-Fa-f]{6})\b\)?", "", cmt, flags=re.I)
        t2 = re.sub(r"\([^)]*(?:deleting|dtor|\?\?_G)[^)]*\)", "", t2, flags=re.I)
        t2 = re.sub(r"\bAllods2\.exe\b.*", "", t2)
        cm2 = ADDR_IN_COMMENT.search(t2)
        if cm2:
            srv = cm2.group(1).upper()
        elif cli and re.search(r"\bAllods2\.exe\b", cmt, re.I):
            srv_none = True
    if srv or cli or srv_none or cli_none:
        base["kind"] = "normal"
        base["srv"], base["cli"], base["srv_none"], base["cli_none"] = srv, cli, srv_none, cli_none
    return base


def parse_headers(files):
    """Yield dicts: file, line, cls, name, ret, params[(type,name)], addr, client_addr, virtual, ..."""
    for path in files:
        try:
            lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
        except OSError:
            continue
        rel = os.path.relpath(path, ROOT)
        stack = []  # (scope_name, brace_depth_at_open, is_namespace)
        pending = None
        depth = 0
        pp_stack = []
        pp_depth_stack = []
        in_block_cmt = False
        i, n = 0, len(lines)
        while i < n:
            raw = lines[i]
            pp = _parse_pp_flags(raw)
            if pp:
                kw, rest = pp
                if kw in ("ifdef", "ifndef", "if"):
                    _pp_push(pp_stack, kw, rest)
                    pp_depth_stack.append((depth, list(stack)))
                elif kw in ("elif", "else") and pp_stack:
                    _pp_push(pp_stack, kw, rest)
                    if pp_depth_stack:
                        depth, stack = pp_depth_stack[-1][0], list(pp_depth_stack[-1][1])
                elif kw == "endif" and pp_stack:
                    _pp_push(pp_stack, kw, rest)
                    if pp_depth_stack:
                        pp_depth_stack.pop()
                i += 1
                continue

            stripped = raw.strip()
            if in_block_cmt:
                if "*/" in stripped:
                    in_block_cmt = False
                i += 1
                continue
            if stripped.startswith("/*"):
                if "*/" not in stripped[2:]:
                    in_block_cmt = True
                i += 1
                continue

            srv_vis, cli_vis, patch_only = _eval_vis(pp_stack)
            line = re.sub(r"/\*.*?\*/", "", _strip_line_comments_and_strings(raw))
            cm = CLASS_RE.match(raw)
            ecm = re.match(r'^\s*extern\s+"C"\s*\{', line)
            if cm:
                stack.append((cm.group(2), depth, cm.group(1) == "namespace"))
            elif ecm:
                stack.append((None, depth, True))
            elif pending and line.lstrip().startswith("{"):
                stack.append((pending[0], depth, pending[1]))
                pending = None
            elif re.match(r'^\s*extern\s+"C"\s*$', line.rstrip()):
                pending = (None, True)
            else:
                om = CLASS_OPEN_RE.match(line.rstrip())
                pending = (om.group(2), om.group(1) == "namespace") if om else None

            in_decl_scope = (not stack) or (depth <= stack[-1][1] + 1)
            joined_code, end_i, tail = line, i, ("//" + raw.split("//", 1)[1] if "//" in raw else "")
            if in_decl_scope:
                code_s = line.strip()
                if DECL_HEAD_RE.match(line):
                    # Join lines until we reach the ending ';' or '{' of this declaration header
                    if not re.search(r"[;{]", line):
                        buf = [line.strip()]
                        j = i + 1
                        while j < n and j < i + 20:
                            lj = lines[j]
                            cj = re.sub(r"/\*.*?\*/", "", _strip_line_comments_and_strings(lj)).strip()
                            buf.append(cj)
                            if re.search(r"[;{]", cj):
                                joined_code = " ".join(buf)
                                end_i = j
                                tail = "//" + lj.split("//", 1)[1] if "//" in lj else ""
                                break
                            j += 1
                elif code_s and "(" in code_s and "{" not in code_s and ";" not in code_s and not code_s.startswith(("typedef", "using", "template", "#", "extern", "friend", "enum", "ASSERT_", "DECLARE_")):
                    # Handle return type on previous line or multi-line params
                    pass

            hm = DECL_HEAD_RE.match(joined_code) if in_decl_scope else None
            if hm and hm.group("name") not in SKIP_NAMES and not re.match(r"\s*(typedef|using|template|#|extern|friend)", joined_code):
                params_s, after_paren = _extract_balanced_decl(joined_code, hm.end())
                decl_ok = False
                if params_s is not None:
                    # Handle initializer list (e.g. `: VisWindow(...) {}`) or `const override = 0;`
                    tail_code = after_paren.strip()
                    m_end = re.match(
                        r"^(const\b)?\s*(?:(?:override|final)\s*)*(?:=\s*(0|default|delete)\s*)?(?::\s*[^;{]*)?([;{])",
                        tail_code,
                    )
                    if m_end:
                        decl_ok = True
                        is_pure = m_end.group(2) == "0"
                        is_defdel = m_end.group(2) in ("default", "delete")
                        end_tok = m_end.group(3)
                if decl_ok:
                    cls, is_ns = (stack[-1][0], stack[-1][2]) if stack else (None, True)
                    name = re.sub(r"\s+", "", hm.group("name"))
                    # Normalize operator spacing cleanly: `operatoruint8_t` stays distinguishable as conversion op
                    raw_name = hm.group("name").strip()
                    is_conv_op = bool(re.match(r"^operator\s+[A-Za-z_]", raw_name))
                    ret_raw, kw = hm.group("ret"), hm.group("kw")
                    ret = norm_type(ret_raw)
                    is_ctor = (cls is not None and name == cls)
                    is_dtor = (cls is not None and name == "~" + cls)
                    if ret or is_ctor or is_dtor:
                        is_inl = (end_tok == "{") or is_defdel
                        if not is_inl and end_i + 1 < n and lines[end_i + 1].split("//")[0].strip().startswith(("{", ":")):
                            is_inl = True
                        meta = parse_va_comment(tail)
                        if not meta["kind"] and i > 0:
                            prev_s = lines[i - 1].strip()
                            if prev_s.startswith("//") and CANONICAL_COMMENT_RE.match(prev_s[2:].strip()):
                                meta = parse_va_comment(prev_s)
                        am = ADDR_IN_NAME.match(name)
                        addr = meta["srv"] or ((am.group(1) or am.group(2)).upper() if am else None)
                        if am and not meta["kind"]:
                            meta["kind"] = "normal"
                            meta["srv"] = addr
                        client_addr = meta["cli"]
                        params = [x for x in (parse_param(p) for p in split_params(params_s)) if x]
                        ccm = re.search(r"__(cdecl|stdcall|fastcall|thiscall)\b", ret_raw + " " + kw)
                        if ccm:
                            cc = "__" + ccm.group(1)
                        elif is_ns or cls is None or "static" in kw:
                            cc = "__cdecl"
                        else:
                            cc = "__thiscall"
                        yield {
                            "file": rel, "line": i + 1, "end_line": end_i + 1,
                            "cls": cls, "name": raw_name if is_conv_op else name,
                            "ret": (cls + " *") if is_ctor else ("void" if is_dtor else ret),
                            "params": params, "ptypes": param_types(params),
                            "addr": addr, "client_addr": client_addr,
                            "virtual": "virtual" in kw, "cc": cc,
                            "plain_ns": is_ns, "ctor": is_ctor, "dtor": is_dtor,
                            "srv_vis": srv_vis, "cli_vis": cli_vis,
                            "patch_only": patch_only, "pure": is_pure, "inline": is_inl,
                            "is_default": is_defdel, "is_move": ("&&" in params_s),
                            "is_conv_op": is_conv_op,
                            "meta_kind": meta["kind"], "raw_cmt": meta["raw"],
                            "split_srv": meta["split_srv"], "split_cli": meta["split_cli"],
                            "bss_srv": meta["bss_srv"], "bss_cli": meta["bss_cli"],
                            "dll_va": meta["dll_va"], "cpp_only": False,
                        }
                        if end_i > i:
                            for u in range(i, end_i + 1):
                                cu = re.sub(r"/\*.*?\*/", "", _strip_line_comments_and_strings(lines[u]))
                                depth += cu.count("{") - cu.count("}")
                                while stack and depth <= stack[-1][1]:
                                    stack.pop()
                            i = end_i + 1
                            continue

            depth += line.count("{") - line.count("}")
            while stack and depth <= stack[-1][1]:
                stack.pop()
            i += 1


CPP_CLS_DEFN = re.compile(
    r"^(?![\s#])(?!return\b)[\w:<>\*&\s,~]*?\b([A-Za-z_]\w*)::(~?[A-Za-z_]\w*|operator\s*(?:\(\)|[+\-*/%^&|~!<>=,\[\]]+))\s*\("
)
CPP_FREE_DEFN = re.compile(
    r"^\s*(?!return\b|typedef\b|using\b|struct\b|class\b|enum\b|namespace\b|extern\b|static_assert\b|ASSERT_|IMPLEMENT_|BEGIN_MESSAGE_MAP|END_MESSAGE_MAP|ON_|#|[A-Za-z_]\w*\s*:)(?:[\w:<>\*&\s,]+?\s+|\s*)(~?[A-Za-z_]\w*)\s*\("
)


def parse_cpp_defs(files):
    """Parse column-0 / namespace-scope .cpp definitions with above-line canonical VA comments."""
    out = []
    for path in files:
        if _is_excluded(path):
            continue
        try:
            lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
        except OSError:
            continue
        rel = os.path.relpath(path, ROOT)
        n, i, pp_stack, pp_depth_stack, ns_stack = len(lines), 0, [], [], []
        depth, in_block_cmt = 0, False
        while i < n:
            raw = lines[i]
            pp = _parse_pp_flags(raw)
            if pp:
                kw, rest = pp
                if kw in ("ifdef", "ifndef", "if"):
                    _pp_push(pp_stack, kw, rest)
                    pp_depth_stack.append((depth, list(ns_stack)))
                elif kw in ("elif", "else") and pp_stack:
                    _pp_push(pp_stack, kw, rest)
                    if pp_depth_stack:
                        depth, ns_stack = pp_depth_stack[-1][0], list(pp_depth_stack[-1][1])
                elif kw == "endif" and pp_stack:
                    _pp_push(pp_stack, kw, rest)
                    if pp_depth_stack:
                        pp_depth_stack.pop()
                i += 1
                continue

            stripped = raw.strip()
            if in_block_cmt:
                if "*/" in stripped:
                    in_block_cmt = False
                i += 1
                continue
            if stripped.startswith("/*"):
                if "*/" not in stripped[2:]:
                    in_block_cmt = True
                i += 1
                continue

            code = re.sub(r"/\*.*?\*/", "", _strip_line_comments_and_strings(raw))
            in_file_scope = (depth == len(ns_stack)) or (len(ns_stack) == 0 and raw and not raw[0].isspace())
            if in_file_scope:
                nsm = re.match(r'^\s*(?:namespace(?:\s+([A-Za-z_]\w*))?|extern\s+"C"\s*)\s*\{', code)
                if nsm:
                    ns_stack.append((nsm.group(1), depth))
                    depth += code.count("{") - code.count("}")
                    while ns_stack and depth <= ns_stack[-1][1]:
                        ns_stack.pop()
                    i += 1
                    continue

            ns_name = next((nm for nm, _ in reversed(ns_stack) if nm), None)
            if in_file_scope and stripped and not stripped.startswith(("//", "/*", "}", "{")):
                m = CPP_CLS_DEFN.match(raw)
                fm = None if m else CPP_FREE_DEFN.match(raw)
                if m or fm:
                    prefix = raw[:fm.start(1)].strip() if fm else ""
                    if fm and (not prefix or prefix.startswith(("class ", "struct ", "enum "))):
                        depth += code.count("{") - code.count("}")
                        while ns_stack and depth <= ns_stack[-1][1]:
                            ns_stack.pop()
                        i += 1
                        continue
                    cls = m.group(1) if m else ns_name
                    name = (m.group(2) if m else fm.group(1)).replace(" ", "")
                    ret_prefix = raw[:m.start(1)] if m else raw[:fm.start(1)]
                    start_i = i
                    pos = m.end() if m else fm.end()
                    k, pdepth, params_s = i, 1, ""
                    while pdepth and k < n and k < i + 15:
                        txt = lines[k].split("//")[0]
                        while pos < len(txt) and pdepth:
                            ch = txt[pos]
                            pdepth += (ch == "(") - (ch == ")")
                            if pdepth:
                                params_s += ch
                            pos += 1
                        if pdepth:
                            params_s += " "
                            k, pos = k + 1, 0
                    if pdepth == 0:
                        brace = None
                        defaulted = bool(re.match(r"^\s*(?:const\s*)?=\s*(?:default|delete)\s*;", lines[k].split("//")[0][pos:]))
                        kk, kpos = k, pos
                        while not defaulted and kk < n and kk < i + 15:
                            txt = lines[kk].split("//")[0][kpos:]
                            if ";" in txt and ("{" not in txt.split(";")[0]):
                                break
                            if "{" in txt:
                                brace = kk
                                break
                            kk, kpos = kk + 1, 0
                        if brace is not None or defaulted:
                            srv_vis, cli_vis, patch_only = _eval_vis(pp_stack)
                            params = [x for x in (parse_param(p) for p in split_params(params_s)) if x]
                            above_line = lines[start_i - 1].strip() if start_i >= 1 and lines[start_i - 1].strip().startswith("//") else ""
                            meta = parse_va_comment(above_line)
                            brace_cmt = lines[brace].split("//", 1)[1].strip() if (brace is not None and "//" in lines[brace]) else ""
                            brace_meta = parse_va_comment("// " + brace_cmt) if brace_cmt else {"kind": None}
                            ccm = re.search(r"__(cdecl|stdcall|fastcall|thiscall)\b", ret_prefix)
                            if ccm:
                                cc = "__" + ccm.group(1)
                            elif cls and not re.match(r"^\s*static\b", raw):
                                cc = "__thiscall"
                            else:
                                cc = "__cdecl"
                            is_ctor = (cls is not None and name == cls)
                            is_dtor = (cls is not None and name == "~" + cls)
                            ret = (cls + " *") if is_ctor else ("void" if is_dtor else norm_type(ret_prefix))
                            out.append({
                                "file": rel, "line": start_i + 1, "brace": (brace + 1 if brace is not None else k + 1),
                                "cls": cls, "plain_ns": (cls is None or cls == ns_name), "name": name,
                                "ret": ret, "params": params, "ptypes": param_types(params),
                                "cc": cc, "virtual": False, "ctor": is_ctor, "dtor": is_dtor,
                                "srv_vis": srv_vis, "cli_vis": cli_vis, "patch_only": patch_only,
                                "is_default": defaulted, "is_move": ("&&" in params_s),
                                "addr": meta["srv"], "client_addr": meta["cli"],
                                "meta_kind": meta["kind"], "raw_cmt": meta["raw"],
                                "split_srv": meta["split_srv"], "split_cli": meta["split_cli"],
                                "dll_va": meta["dll_va"], "brace_meta_kind": brace_meta["kind"],
                                "cpp_only": True,
                            })
                            end_scan = brace if brace is not None else k
                            for u in range(start_i, end_scan + 1):
                                cu = re.sub(r"/\*.*?\*/", "", _strip_line_comments_and_strings(lines[u]))
                                depth += cu.count("{") - cu.count("}")
                                while ns_stack and depth <= ns_stack[-1][1]:
                                    ns_stack.pop()
                            i = end_scan + 1
                            continue

            depth += code.count("{") - code.count("}")
            while ns_stack and depth <= ns_stack[-1][1]:
                ns_stack.pop()
            i += 1
    return out


# MFC container placeholder sizes (verified in our build; pointer parameters only)
TEMPLATE_FAMILY_SIZES = (
    ("CArray<", 0x14), ("CWordArray", 0x14), ("CDWordArray", 0x14),
    ("CList<", 0x1c), ("CMap<", 0x1c), ("CFileException", 0x40),
)


def precreate_types(todo):
    """Create missing MFC container placeholder structs in the target Ghidra program."""
    names = set()
    for d in todo:
        for t in [d["ret"]] + [t for t, _ in d["params"]]:
            for m in re.finditer(r"[A-Za-z_]\w*(?:<[^;()]*?>)?", t):
                n = m.group(0)
                if any(n.startswith(pfx) for pfx, _ in TEMPLATE_FAMILY_SIZES):
                    names.add(n)
    made = 0
    for n in sorted(names):
        size = next(s for p, s in TEMPLATE_FAMILY_SIZES if n.startswith(p))
        try:
            call("struct-show", {"name": n})
            continue
        except SystemExit:
            pass
        try:
            call("struct-create", {"name": n, "size": size})
            made += 1
        except SystemExit:
            pass
    print(f"container placeholder types pre-created: {made} of {len(names)} needed")


def _norm_t_loose(t):
    t = re.sub(r"/\*.*?\*/", "", t)
    t = re.sub(r"\bconst\b", "", t.strip())
    t = re.sub(r"\s+", "", t).replace("&", "*")
    aliases = {
        "int": "int32_t", "signedint": "int32_t", "LONG": "int32_t", "BOOL": "int",
        "unsignedint": "uint32_t", "UINT": "uint32_t", "DWORD": "uint32_t", "ULONG": "uint32_t", "size_t": "uint32_t",
        "unsignedshort": "uint16_t", "WORD": "uint16_t", "USHORT": "uint16_t",
        "short": "int16_t", "signedshort": "int16_t",
        "unsignedchar": "uint8_t", "BYTE": "uint8_t", "byte": "uint8_t", "uchar": "uint8_t",
        "signedchar": "int8_t",
        "LPCSTR": "char*", "LPCTSTR": "char*", "LPSTR": "char*", "LPTSTR": "char*",
        "LPVOID": "void*", "LPCVOID": "void*",
    }
    return aliases.get(t, t)


def match_decls_and_defs(decls, defs):
    by_cn = defaultdict(list)
    for i, d in enumerate(decls):
        if not d["patch_only"] and not d["is_move"]:
            by_cn[(d["cls"] or "None", d["name"])].append(i)

    def_to_decl, decl_to_defs = {}, defaultdict(list)
    for df_idx, df in enumerate(defs):
        if df["patch_only"] or df["is_move"]:
            continue
        cls, name = df["cls"] or "None", df["name"]
        cands = [di for di in by_cn.get((cls, name), []) if not decls[di]["pure"]]
        lk = tuple(_norm_t_loose(p) for p in df["ptypes"])
        exact = [di for di in cands if tuple(_norm_t_loose(p) for p in decls[di]["ptypes"]) == lk]
        if len(exact) == 1:
            def_to_decl[df_idx] = exact[0]
            decl_to_defs[exact[0]].append(df_idx)
        elif len(exact) > 1:
            vis_m = [di for di in exact if (decls[di]["srv_vis"], decls[di]["cli_vis"]) == (df["srv_vis"], df["cli_vis"])]
            if len(vis_m) == 1:
                def_to_decl[df_idx] = vis_m[0]
                decl_to_defs[vis_m[0]].append(df_idx)

    # Also attach PATCH override definitions alongside retail twins in same .cpp (e.g. SaveScreenshot)
    for df_idx, df in enumerate(defs):
        if not df["patch_only"] or df["is_move"] or not df["meta_kind"]:
            continue
        cls, name = df["cls"] or "None", df["name"]
        cands = [di for di in by_cn.get((cls, name), []) if not decls[di]["pure"]]
        lk = tuple(_norm_t_loose(p) for p in df["ptypes"])
        exact = [di for di in cands if tuple(_norm_t_loose(p) for p in decls[di]["ptypes"]) == lk]
        if len(exact) == 1 and any(defs[x]["file"] == df["file"] for x in decl_to_defs.get(exact[0], [])):
            def_to_decl[df_idx] = exact[0]
            decl_to_defs[exact[0]].append(df_idx)

    return def_to_decl, decl_to_defs


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


def _gh_python(code):
    res = call("python", {"code": code})
    return res.get("output", "") if isinstance(res, dict) else str(res)


def _va_tuple(item):
    k = item["meta_kind"]
    if not k:
        return None
    if k == "normal":
        return ("normal", (item["addr"] or "").lower() or None, (item["client_addr"] or "").lower() or None)
    if k in ("split", "inlined"):
        return (k, (item["split_srv"] or "").lower() or None, (item["split_cli"] or "").lower() or None)
    if k in ("fnptr", "scenario_dll", "scenario_split"):
        return ("scenario_dll", (item["dll_va"] or "").lower())
    return None


def _run_verify(prog_label, decls, defs, def_to_decl, decl_to_defs):
    want_cli = prog_label == "client"
    icf_map = ICF_ALLOWLIST.get(prog_label, {})
    fmt_errors, parity_errors, missing_va = [], [], []
    va_owners = defaultdict(list)  # target_va_lower -> list of (qname, loc, is_dtor, is_conv_op)

    for di, d in enumerate(decls):
        if d["patch_only"] or d["is_move"]:
            continue
        cls = d["cls"] or "None"
        qname = f"{cls}::{d['name']}" if d["cls"] else d["name"]
        if _is_non_retail(cls, d["name"], d["ptypes"]):
            continue
        if d["pure"] or (d["is_default"] and not d["meta_kind"]):
            continue
        if d["inline"] and not d["meta_kind"] and not d["virtual"]:
            continue
        cmt = d["raw_cmt"]
        if not cmt or not CANONICAL_COMMENT_RE.match(cmt):
            fmt_errors.append(("H_FMT", f"{d['file']}:{d['line']}", qname, cmt))
        vt = _va_tuple(d)
        if not vt:
            missing_va.append(("H_MISSING", f"{d['file']}:{d['line']}", qname))
        for df_idx in decl_to_defs.get(di, []):
            df = defs[df_idx]
            if not df["raw_cmt"] or not CANONICAL_COMMENT_RE.match(df["raw_cmt"]):
                fmt_errors.append(("CPP_FMT", f"{df['file']}:{df['line']}", qname, df["raw_cmt"]))
            if df["brace_meta_kind"] is not None:
                fmt_errors.append(("CPP_BRACE_VA", f"{df['file']}:{df['brace']}", qname, ""))
            if _va_tuple(df) != vt:
                parity_errors.append((f"{d['file']}:{d['line']}", f"{df['file']}:{df['line']}", vt, _va_tuple(df)))

        vis = d["cli_vis"] if want_cli else d["srv_vis"]
        tva = (d["client_addr"] if want_cli else d["addr"])
        if vis and d["meta_kind"] == "normal" and tva:
            va_owners[tva.lower()].append((qname, f"{d['file']}:{d['line']}", d["dtor"], d["is_conv_op"], cls, d["name"]))

    for df_idx, df in enumerate(defs):
        if df["patch_only"] or df["is_move"] or df_idx in def_to_decl:
            continue
        cls = df["cls"] or "None"
        qname = f"{cls}::{df['name']}" if df["cls"] else df["name"]
        if _is_non_retail(cls, df["name"], df["ptypes"]):
            continue
        if not df["raw_cmt"] or not CANONICAL_COMMENT_RE.match(df["raw_cmt"]):
            fmt_errors.append(("CPP_ONLY_FMT", f"{df['file']}:{df['line']}", qname, df["raw_cmt"]))
        if df["brace_meta_kind"] is not None:
            fmt_errors.append(("CPP_ONLY_BRACE_VA", f"{df['file']}:{df['brace']}", qname, ""))
        vt = _va_tuple(df)
        if not vt:
            missing_va.append(("CPP_ONLY_MISSING", f"{df['file']}:{df['line']}", qname))
        vis = df["cli_vis"] if want_cli else df["srv_vis"]
        tva = (df["client_addr"] if want_cli else df["addr"])
        if vis and df["meta_kind"] == "normal" and tva:
            va_owners[tva.lower()].append((qname, f"{df['file']}:{df['line']}", df["dtor"], False, cls, df["name"]))

    dup_errors = []
    for va, entries in sorted(va_owners.items()):
        if len(entries) > 1:
            qnames = frozenset(e[0] for e in entries)
            if icf_map.get(va) != qnames:
                dup_errors.append((va, [(e[0], e[1]) for e in entries]))

    all_vas = sorted(int(va, 16) for va in va_owners)
    gh_code = r"""
import json
fm = currentProgram.getFunctionManager()
listing = currentProgram.getListing()
rm = currentProgram.getReferenceManager()
mem = currentProgram.getMemory()
af = currentProgram.getAddressFactory()
def addr(v): return af.getDefaultAddressSpace().getAddress(v)
out = {}
for v in __VAS__:
    a = addr(v)
    f = fm.getFunctionAt(a)
    if not f:
        out["{:06x}".format(v)] = {"exists": False}
        continue
    blk = mem.getBlock(a)
    bname = blk.getName() if blk else "?"
    ins = listing.getInstructions(f.getBody(), True)
    rets = []
    while ins.hasNext():
        s = str(ins.next())
        if s.startswith("RET"): rets.append(s)
    has_rdata = False
    for r in rm.getReferencesTo(a):
        rb = mem.getBlock(r.getFromAddress())
        if rb and rb.getName() == ".rdata":
            has_rdata = True
            break
    out["{:06x}".format(v)] = {
        "exists": True, "block": bname, "rets": rets, "has_rdata": has_rdata,
        "full": str(f.getName(True)), "short": str(f.getName()),
    }
print("VERIFY_JSON:" + json.dumps(out))
""".replace("__VAS__", repr(all_vas))
    txt = _gh_python(gh_code)
    live = None
    for ln in txt.splitlines():
        if ln.startswith("VERIFY_JSON:"):
            live = json.loads(ln[len("VERIFY_JSON:"):])
            break
    if live is None:
        sys.exit(f"ERROR: no VERIFY_JSON from Ghidra ({prog_label}): {txt[:200]}")

    entry_errors, dtor_errors, name_mismatches = [], [], []
    for va, entries in sorted(va_owners.items()):
        st = live.get(va, {})
        qname, loc, is_dtor, is_conv, cls, name = entries[0]
        if not st.get("exists") or st.get("block") != ".text":
            entry_errors.append((va, qname, loc, st))
            continue
        if is_dtor:
            rets = st.get("rets", [])
            if "RET" not in rets or any(r.startswith("RET ") for r in rets) or st.get("has_rdata"):
                dtor_errors.append((va, qname, loc, {"rets": rets, "has_rdata": st.get("has_rdata")}))
        if not is_conv:
            expected_names = {e[0] for e in entries}
            # Allow Ghidra namespace/class match or short-name match if plain global/anon-ns
            actual = st.get("full", "")
            short = st.get("short", "")
            ok_name = (actual in expected_names) or any(e[4] == "None" and short == e[5] for e in entries)
            if not ok_name:
                name_mismatches.append((va, sorted(expected_names), actual, loc))

    print(f"[{prog_label}] VERIFY: normal_vas={len(va_owners)} "
          f"fmt_err={len(fmt_errors)} missing_va={len(missing_va)} parity_err={len(parity_errors)} "
          f"dup_err={len(dup_errors)} entry_err={len(entry_errors)} dtor_err={len(dtor_errors)} "
          f"name_mismatch={len(name_mismatches)}")
    for lbl, arr in [("FMT", fmt_errors), ("MISSING", missing_va), ("PARITY", parity_errors),
                     ("DUP", dup_errors), ("ENTRY", entry_errors), ("DTOR", dtor_errors),
                     ("NAME_MISMATCH", name_mismatches)]:
        for item in arr[:20]:
            print(f"  {lbl}: {item}")
        if len(arr) > 20:
            print(f"  {lbl}: ... +{len(arr) - 20} more")

    total_err = (len(fmt_errors) + len(missing_va) + len(parity_errors) + len(dup_errors) +
                 len(entry_errors) + len(dtor_errors) + len(name_mismatches))
    if total_err:
        sys.exit(1)
    print(f"[{prog_label}] VERIFY PASSED (0 failures)!")


def main():
    global PORT, PROGRAM
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--verify", action="store_true", help="verify VA format/parity and live Ghidra state")
    ap.add_argument("--class", dest="cls", help="only this class")
    ap.add_argument("--addr", help="only this address (hex)")
    ap.add_argument("--file", help="only declarations from this header")
    ap.add_argument("--create-classes", action="store_true", help="create empty structs for unknown classes")
    ap.add_argument("--create-types", action="store_true", help="create placeholder structs for unknown pointer types")
    ap.add_argument("--port", type=int, default=PORT)
    sel = ap.add_mutually_exclusive_group()
    sel.add_argument("--server", action="store_true", help="apply to the server program (a2serv6.exe_new)")
    sel.add_argument("--client", action="store_true",
                     help="apply to the client program (allods2.exe) at each declaration's "
                          "`(client: <va>)` marker; declarations without one are skipped")
    sel.add_argument("--program", help="program by name (server/client/...) - deliberately no env var: "
                                       "bulk pushes should be explicit")
    a = ap.parse_args()
    PORT = a.port
    PROGRAM = "server" if a.server else "client" if a.client else a.program
    target_label = PROGRAM or "server"

    headers = [a.file] if a.file else [h for h in sorted(glob.glob(os.path.join(SRC, "*.h"))) if not _is_excluded(h)]
    cpps = [c for c in sorted(glob.glob(os.path.join(SRC, "*.cpp"))) if not _is_excluded(c)]

    decls = list(parse_headers(headers))
    defs = parse_cpp_defs(cpps) if not a.file else []
    def_to_decl, decl_to_defs = match_decls_and_defs(decls, defs)

    if a.verify:
        _run_verify(target_label, decls, defs, def_to_decl, decl_to_defs)
        return

    # Assemble push list: in-scope retail header declarations + in-scope .cpp-only retail definitions
    candidates = [d for d in decls if not d["patch_only"] and not d["is_move"]]
    for df_idx, df in enumerate(defs):
        if df["patch_only"] or df["is_move"] or df_idx in def_to_decl:
            continue
        candidates.append(df)

    want_cli = (target_label == "client")
    icf_map = ICF_ALLOWLIST.get(target_label, {})
    icf_seen = set()

    filtered = []
    for d in candidates:
        if not (d["cli_vis"] if want_cli else d["srv_vis"]):
            continue
        va = d["client_addr"] if want_cli else d["addr"]
        d["_push_addr"] = va
        filtered.append(d)

    seen, todo, skipped = set(), [], []
    for d in filtered:
        if a.cls and d["cls"] != a.cls:
            continue
        if a.addr and (d["_push_addr"] or "").upper() != a.addr.upper().removeprefix("0X"):
            continue
        if _is_non_retail(d["cls"], d["name"], d["ptypes"]):
            skipped.append((d, "non-retail"))
            continue
        if not d["_push_addr"] or d["meta_kind"] != "normal":
            skipped.append((d, "no address" if not d["meta_kind"] or d["meta_kind"] == "normal" else d["meta_kind"]))
            continue
        if d.get("is_conv_op") or ("operator" in d["name"] and " " in d["name"]):
            skipped.append((d, "conversion operator"))
            continue
        va_l = d["_push_addr"].lower()
        qname = f"{d['cls']}::{d['name']}" if d["cls"] else d["name"]
        if d["_push_addr"] in seen:
            if qname in icf_map.get(va_l, ()):
                skipped.append((d, "icf allowlist"))
            else:
                skipped.append((d, "duplicate address"))
            continue
        seen.add(d["_push_addr"])
        if qname in icf_map.get(va_l, ()):
            icf_seen.add(va_l)
        todo.append(d)

    print(f"[{target_label}] {len(filtered)} declarations/defs visible, {len(todo)} with addresses to apply, {len(skipped)} skipped")
    if a.dry_run:
        for d in todo:
            ps = ", ".join(f"{t} {n}" if n else t for t, n in d["params"])
            scope = (d["cls"] + "::") if d["cls"] else ""
            tag = " [cpp-only]" if d.get("cpp_only") else ""
            print(f"  {d['_push_addr']}  {d['ret']} {d['cc']} {scope}{d['name']}({ps})  [{d['file']}:{d['line']}]{tag}")
        for d, why in skipped:
            if why not in ("no address", "split", "inlined", "fnptr", "scenario_dll", "scenario_split"):
                print(f"  SKIP ({why}) {d['cls']}::{d['name']} [{d['file']}:{d['line']}]")
        return

    if a.create_types:
        precreate_types(todo)

    ops = []
    for d in todo:
        ops.append({"op": "set-proto", "args": {
            "target": d["_push_addr"], "ret": _proto_type(d["ret"]),
            "params": [f"{_proto_type(t)} {n}" if n else _proto_type(t) for t, n in d["params"]],
            "cc": d["cc"], "cls": d["cls"] or "", "name": d["name"], "plain_ns": d["plain_ns"],
            "keep_better": True, "split": True,
            "create": a.create_classes or a.create_types}})
        if d["virtual"]:
            ops.append({"op": "set-comment", "args": {"target": d["_push_addr"], "text": "virtual"}})
        va_l = d["_push_addr"].lower()
        if va_l in icf_map:
            ops.append({"op": "set-comment", "args": {
                "target": d["_push_addr"],
                "text": "ICF: " + ", ".join(sorted(icf_map[va_l]))}})

    ok = err = 0
    for i in range(0, len(ops), 200):
        res = call("batch", {"ops": ops[i:i + 200]})
        for line in res.splitlines():
            if line.startswith("ERR"):
                err += 1
                print(line)
            else:
                ok += 1
    print(f"[{target_label}] applied {ok} ops, {err} errors")
    if err:
        sys.exit(1)


if __name__ == "__main__":
    main()
