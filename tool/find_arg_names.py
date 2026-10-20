#!/usr/bin/env python3
"""Inventory placeholder argument names (argN/paramN) in src/*.h,src/*.cpp.

Classifies each occurrence as:
  SIG   - part of a function signature (declaration or definition)
  BODY  - bare identifier use inside a function body
  FIELD - member access (this->, ->, .) or struct field declaration
  COMMENT - inside a comment
Prints groups per function definition.
"""
import re
import sys
from collections import defaultdict

ROOT = r"C:\Users\ikacn\Desktop\stuff\a2\assembly\src"
PLACEHOLDER = re.compile(r"\b(arg\d+|param\d+)\b")
FILES = [
    "gameobj.h", "gameobj.cpp", "ingame.cpp", "server.h", "server.cpp",
    "net.h", "net.cpp", "packet.h", "packet.cpp", "main_window.h",
    "main_window.cpp", "world.h", "world.cpp", "logic.h", "gfx.h",
    "game_app.h", "shelf.h", "shelf.cpp",
]


def classify(path):
    fname = path.replace("\\", "/").split("/")[-1]
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    out = []  # (lineno, line, kind, names)
    for i, line in enumerate(lines, 1):
        if not PLACEHOLDER.search(line):
            continue
        names = PLACEHOLDER.findall(line)
        stripped = line.strip()
        if stripped.startswith("//") or stripped.startswith("*"):
            out.append((i, line.rstrip(), "COMMENT", names))
            continue
        # remove member accesses and struct-field decls for classification
        probe = re.sub(r"(this->|->|\.)\b(?:arg|param)\d+\b", "", line)
        # field declaration like "uint32_t arg1;" / "int32_t param1;" at namespace/class scope
        is_field_decl = re.search(r"\b(?:u?int(?:8|16|32|64)_t|DWORD|WORD|BYTE)\s+(?:arg|param)\d+\s*(=|;|,)", line)
        if "->" in line and re.search(r"(->|\.)\s*(?:arg|param)\d+\b", line) and probe == line.replace("\n", ""):
            pass
        if is_field_decl and "(" not in line.split(is_field_decl.group(1))[0]:
            # "uint32_t arg1;" style line with no '(' before the decl
            if "(" not in line or re.match(r"\s*(u?int(?:8|16|32|64)_t|DWORD|WORD|BYTE)\s", line):
                out.append((i, line.rstrip(), "FIELD", names))
                continue
        if probe != line:
            out.append((i, line.rstrip(), "FIELD", names))
            continue
        # signature? contains '(' and ')' and the placeholder is preceded by a type token
        if "(" in line and ")" in line:
            sig_part = line
            if re.search(r"[\w*&]\s+(?:arg|param)\d+\b", line):
                out.append((i, line.rstrip(), "SIG", names))
                continue
        out.append((i, line.rstrip(), "BODY", names))
    return out, fname


def main():
    groups = defaultdict(list)
    for f in FILES:
        path = f"{ROOT}\\{f}"
        try:
            items, fname = classify(path)
        except FileNotFoundError:
            continue
        for lineno, line, kind, names in items:
            groups[(fname, kind)].append((lineno, line, names))
    for (fname, kind) in sorted(groups):
        items = groups[(fname, kind)]
        print(f"=== {fname} [{kind}] ({len(items)} lines) ===")
        for lineno, line, names in items:
            print(f"{lineno}: {line.strip()[:150]}")
        print()


if __name__ == "__main__":
    main()
