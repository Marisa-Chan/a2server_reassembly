"""Scan headers for class declarations of sub_*/FUN_*/VMethod* placeholders.

Prints one line per (class, name): `Class::Name  (header:line)`.
Usage: python tool/scan_vmethods.py [ClassFilter]
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PAT = re.compile(r'\b(sub_[0-9A-Fa-f]+|FUN_[0-9A-Fa-f]+|VMethod\d+)\b\s*\(')


def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
    return text


def classes_and_spans(text):
    """Yield (class_name, start, end) for top-level class/struct bodies."""
    for m in re.finditer(r'^[ \t]*(?:class|struct)\s+(\w+)[^;{]*\{', text, re.M):
        start = m.end() - 1
        depth = 0
        i = start
        while i < len(text):
            if text[i] == '{':
                depth += 1
            elif text[i] == '}':
                depth -= 1
                if depth == 0:
                    break
            i += 1
        yield m.group(1), start, i


def main():
    filt = sys.argv[1] if len(sys.argv) > 1 else None
    for f in sorted((ROOT / 'src').glob('*.h')):
        raw = f.read_text(encoding='utf-8', errors='replace')
        text = strip_comments(raw)
        # map char offsets to line numbers
        lines = raw.splitlines()
        off2line = [0] * (len(text) + 1)
        pos = 0
        for ln, line in enumerate(lines, 1):
            for k in range(len(line) + 1):
                if pos + k < len(off2line):
                    off2line[pos + k] = ln
            pos += len(line) + 1

        for cls, s, e in classes_and_spans(text):
            if filt and filt != cls:
                continue
            body = text[s:e]
            for m in PAT.finditer(body):
                line = off2line[s + m.start()] if s + m.start() < len(off2line) else '?'
                print(f"{cls}::{m.group(1)}  ({f.name}:{line})")


if __name__ == '__main__':
    main()
