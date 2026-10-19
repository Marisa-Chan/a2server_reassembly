#!/usr/bin/env python3
"""Join opening curly braces onto the previous line (Allman -> K&R).

    if (cond)
    {
becomes
    if (cond) {

Rules:
  * A line whose only code token is `{` (optionally followed by a comment)
    is joined onto the previous line, when that line ends like a block
    opener: an identifier (`else`, `do`, `try`, a struct/class name) or
    `)` (function signature, if/for/while/switch) or `:` (case label).
  * Variable/aggregate initializers are left as-is: the previous line ends
    with `=`, `,`, `(`, `[`, `]` -> skip.
  * Naked scope blocks are left as-is: previous line ends with `;`, `{` or
    `}` -> skip.
  * A `{` directly after a preprocessor line (`#ifdef` etc.) is skipped.
  * Trailing `//` comments survive: they end up behind the joined `{`.
  * Never touches dplay.h / dplobby.h (vendored SDK headers).

A post-transform self-check verifies the code token stream is unchanged.

Usage:
  python tool/join_braces.py            # dry run, per-file stats + skips
  python tool/join_braces.py --apply    # rewrite files
  python tool/join_braces.py --diff x.cpp  # unified diff for one file
"""

import difflib
import glob
import os
import re
import sys

TOOL_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(TOOL_DIR)
sys.path.insert(0, TOOL_DIR)
from add_braces import tokenize  # noqa: E402

EXCLUDE = {'dplay.h', 'dplobby.h'}

FILES = [f for f in glob.glob(os.path.join(ROOT, 'src', '*.[ch]*'))
         if os.path.basename(f) not in EXCLUDE and not f.endswith('.c')]
FILES.append(os.path.join(ROOT, 'asm_crt_test', 'testAsmInit.cpp'))

JOIN_LAST_TOKENS = None  # see token_class()
SKIP_LAST_TOKENS = set('=([,;]}{.?~!&|+-*/%^<')  # noqa: SIM - chars


def token_class(tok_text):
    """'join' if a lone `{` may attach to a line ending in this token."""
    if re.fullmatch(r'[A-Za-z_]\w*', tok_text):
        return 'join'          # else / do / try / case-keyword / type name
    if tok_text in (')', ':', '>'):
        return 'join'          # signature / control head / case label /
                               # template argument list
    if tok_text in SKIP_LAST_TOKENS:
        return 'skip'
    return 'skip'              # anything else: be conservative


def line_spans(text):
    """[(line_start, line_end_exclusive)] over text."""
    spans = []
    start = 0
    for line in text.split('\n'):
        spans.append((start, start + len(line)))
        start += len(line) + 1
    return spans


def analyze(text):
    """Return (lines, candidates, skips).

    candidates/skips: (brace_line_idx, prev_line_idx, reason) with reason
    only set for skips."""
    lines = text.split('\n')
    spans = line_spans(text)
    assert len(spans) == len(lines)

    # code tokens per line (tokens are emitted in order -> monotonic walk)
    toks_per_line = [[] for _ in lines]
    toks_all, _pp = tokenize(text)
    li = 0
    for s, e in toks_all:
        while not (spans[li][0] <= s < spans[li][1]):
            li += 1
        toks_per_line[li].append((s, e, text[s:e]))

    candidates = []
    skips = []
    for i, toks in enumerate(toks_per_line):
        if i == 0 or [t[2] for t in toks] != ['{']:
            continue
        prev = i - 1
        if not lines[prev].strip():
            skips.append((i, prev, 'prev line blank'))
            continue
        if lines[prev].lstrip().startswith('#'):
            skips.append((i, prev, 'prev line is preprocessor'))
            continue
        if lines[prev].rstrip().endswith('\\'):
            skips.append((i, prev, 'prev line is macro continuation'))
            continue
        ptoks = toks_per_line[prev]
        if not ptoks:
            skips.append((i, prev, 'prev line has no code tokens (comment)'))
            continue
        last = ptoks[-1][2]
        kind = token_class(last)
        if kind != 'join':
            skips.append((i, prev, f'prev ends with {last!r} '
                                   f'(initializer or naked scope)'))
            continue
        candidates.append((i, prev, None))
    return lines, candidates, skips


def transform(text, fname):
    """Return new text or None if nothing changed."""
    lines, candidates, _ = analyze(text)
    if not candidates:
        return None
    out = list(lines)
    # bottom-up so line indices stay valid
    for brace_i, prev_i, _ in sorted(candidates, reverse=True):
        prev = out[prev_i]
        brace_line = out[brace_i]
        # split prev line at its trailing comment (text after last token)
        prev_eol = prev.endswith('\r')
        core = prev[:-1] if prev_eol else prev
        m = re.search(r'(?<![:\w])//', core)
        if m and core[m.start():].strip().startswith('//'):
            code, comment = core[:m.start()].rstrip(), core[m.start():].strip()
        else:
            code, comment = core.rstrip(), ''
        # trailing comment on the brace line itself
        bm = re.match(r'\s*\{\s*(//.*)$', brace_line)
        if bm:
            comment = (comment + ' ' + bm.group(1).strip()).strip()
        joined = code + ' {' + ((' ' + comment) if comment else '')
        out[prev_i] = joined + ('\r' if prev_eol else '')
        del out[brace_i]
    new = '\n'.join(out)
    # self-check: token stream must be identical
    old_toks = [text[s:e] for s, e in tokenize(text)[0]]
    new_toks = [new[s:e] for s, e in tokenize(new)[0]]
    if old_toks != new_toks:
        raise SystemExit(f'{fname}: token stream changed, aborting file')
    return new


def main():
    apply_mode = '--apply' in sys.argv
    diff_one = None
    for a in sys.argv[1:]:
        if a.startswith('--diff'):
            continue
        if not a.startswith('--'):
            diff_one = a

    total_joins = total_skips = 0
    files_changed = 0
    for fname in FILES:
        rel = os.path.relpath(fname, ROOT)
        if not os.path.exists(fname):
            continue
        raw = open(fname, 'rb').read()
        text = raw.decode('latin-1')
        if diff_one and diff_one not in rel:
            continue
        _, candidates, skips = analyze(text)
        new = transform(text, rel)
        if new is None:
            if candidates or skips:
                print(f'{rel}: 0 joins ({len(skips)} skipped)')
            continue
        if diff_one:
            print(''.join(difflib.unified_diff(
                text.splitlines(True), new.splitlines(True),
                fromfile=rel, tofile=rel + ' (joined)')))
        elif apply_mode:
            open(fname, 'wb').write(new.encode('latin-1'))
        files_changed += 1
        total_joins += len(candidates)
        total_skips += len(skips)
        print(f'{rel}: {len(candidates)} joins, {len(skips)} skipped')
        all_lines = text.split('\n')
        for i, prev, reason in skips:
            print(f'    skipped line {i + 1} (after {prev + 1}): '
                  f'{reason}: {all_lines[i].strip()}')

    print(f'\nTOTAL: {total_joins} joins, {total_skips} skipped, '
          f'{files_changed} files'
          f'{" (dry run)" if not apply_mode and not diff_one else ""}')


if __name__ == '__main__':
    main()
