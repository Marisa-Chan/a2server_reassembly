#!/usr/bin/env python3
"""Add curly braces around single-statement if/else/for/while/do bodies.

Scans src/*.cpp and src/*.h, finds unbraced control-flow bodies and rewrites
them in place:

    if (cond)
        stmt;
becomes
    if (cond) {
        stmt;
    }

Style rules:
  * `} else {` — an else clause is joined onto the closing brace line
    (also normalizes pre-existing `}\\nelse` / `}\\nelse\\n{` layouts).
  * Blank lines between a body statement and the next token end up AFTER
    the closing brace, not inside it.

`else if` chains keep their else unbraced. Comments, string/char literals
and preprocessor lines are skipped; preprocessor conditionals are handled
with a first-branch-wins policy and any brace pair straddling a preprocessor
boundary is left untouched. A post-transform self-check verifies that only
braces/whitespace were added and that nothing else moved.

Usage:
  python tool/add_braces.py                 # dry run, per-file stats
  python tool/add_braces.py --apply         # rewrite files
  python tool/add_braces.py --limit packet  # only files whose name contains 'packet'
  python tool/add_braces.py --diff packet.cpp  # print unified diff instead of writing
"""

import difflib
import glob
import os
import re
import sys

SRC_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'src')
EXCLUDE = {'dplay.h', 'dplobby.h'}  # vendored SDK headers, leave untouched

CONTROL = ('if', 'for', 'while')

# A single-statement body longer than this is suspicious (macro without `;`
# swallowing the next statement) -> abort the file for manual review.
MAX_BODY_TOKENS = 120
MAX_BODY_LINES = 40


class Abort(Exception):
    pass


def tokenize(text):
    """Return a list of (start, end) offsets of code tokens (words, numbers,
    single punctuation chars). Comments, string/char literals and
    preprocessor directives are skipped entirely.

    Preprocessor conditionals are handled with a first-branch-wins policy:
    after `#if/#ifdef/#ifndef` only the first branch is tokenized; `#else` /
    `#elif` branches are skipped. This keeps brace pairing consistent even
    in MFC sources where braces pair across `#else` boundaries."""
    toks = []
    pp_ranges = []  # (start, end) of every preprocessor directive
    cond_stack = []  # 'FIRST' = tokenizing the selected branch, 'SKIP' = skipping
    n = len(text)
    i = 0
    while i < n:
        c = text[i]
        if c in ' \t\r\n\f\v':
            i += 1
            continue
        if c == '#':
            # Only treat as a directive when `#` starts the line.
            ls = text.rfind('\n', 0, i) + 1
            if text[ls:i].strip() != '':
                toks.append((i, i + 1))
                i += 1
                continue
            j = i
            while j < n:
                nl = text.find('\n', j)
                if nl < 0:
                    j = n
                    break
                k = nl - 1
                if k >= 0 and text[k] == '\r':
                    k -= 1
                if k >= 0 and text[k] == '\\':
                    j = nl + 1
                    continue
                j = nl
                break
            pp_ranges.append((i, j))
            directive = text[i:j]
            m = re.match(r'#\s*([A-Za-z_]+)', directive)
            kw = m.group(1) if m else ''
            if kw in ('if', 'ifdef', 'ifndef'):
                cond_stack.append('FIRST' if all(s == 'FIRST' for s in cond_stack) else 'SKIP')
            elif kw in ('else', 'elif'):
                if cond_stack and cond_stack[-1] == 'FIRST':
                    cond_stack[-1] = 'SKIP'
            elif kw == 'endif':
                if cond_stack:
                    cond_stack.pop()
            i = j
            continue
        if any(s == 'SKIP' for s in cond_stack):
            # Inactive preprocessor branch: skip code without parsing it.
            i += 1
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '/':
            j = text.find('\n', i)
            i = n if j < 0 else j
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '*':
            j = text.find('*/', i + 2)
            i = n if j < 0 else j + 2
            continue
        if c == '"' or c == "'":
            q = c
            j = i + 1
            while j < n:
                if text[j] == '\\':
                    j += 2
                    continue
                if text[j] == q:
                    j += 1
                    break
                j += 1
            i = j
            continue
        if c.isalpha() or c in '_$':
            j = i + 1
            while j < n and (text[j].isalnum() or text[j] in '_$'):
                j += 1
            toks.append((i, j))
            i = j
            continue
        if c.isdigit():
            j = i + 1
            while j < n and (text[j].isalnum() or text[j] in '._'):
                j += 1
            toks.append((i, j))
            i = j
            continue
        toks.append((i, i + 1))
        i += 1
    return toks, pp_ranges


class FileFixer:
    def __init__(self, path):
        self.path = path
        with open(path, 'rb') as f:
            data = f.read()
        self.bom = data.startswith(b'\xef\xbb\xbf')
        if self.bom:
            data = data[3:]
        self.text = data.decode('utf-8', errors='surrogateescape')
        self.nl = '\r\n' if '\r\n' in self.text else '\n'
        self.toks, self.pp_ranges = tokenize(self.text)
        self.inserts = []       # (pos, text)
        self.replacements = []  # (start, end, text)
        self.stats = {'braced': 0, 'else_if': 0, 'empty': 0, 'same_line': 0,
                      'close_midline': 0, 'do': 0, 'pp_skip': 0, 'else_join': 0}
        tab_lines = len(re.findall(r'(?m)^\t', self.text))
        sp_lines = len(re.findall(r'(?m)^    ', self.text))
        self.unit = '\t' if tab_lines > sp_lines else '    '
        self.line_offsets = [0] + [m.end() for m in re.finditer('\n', self.text)]

    # ---- token helpers -------------------------------------------------

    def tok_text(self, idx):
        s, e = self.toks[idx]
        return self.text[s:e]

    def line_of(self, pos):
        return self.text.count('\n', 0, pos)

    def line_start(self, pos):
        return self.text.rfind('\n', 0, pos) + 1

    def line_indent(self, pos):
        ls = self.line_start(pos)
        m = re.match(r'[ \t]*', self.text[ls:])
        return self.text[ls:ls + m.end()]

    def first_nonspace(self, pos):
        ls = self.line_start(pos)
        m = re.match(r'[ \t]*', self.text[ls:])
        return ls + m.end()

    def match_paren(self, idx):
        depth = 0
        j = idx
        while j < len(self.toks):
            t = self.tok_text(j)
            if t == '(':
                depth += 1
            elif t == ')':
                depth -= 1
                if depth == 0:
                    return j
            j += 1
        raise Abort('unbalanced ( at token %d' % idx)

    def match_brace(self, idx):
        depth = 0
        j = idx
        while j < len(self.toks):
            t = self.tok_text(j)
            if t == '{':
                depth += 1
            elif t == '}':
                depth -= 1
                if depth == 0:
                    return j
            j += 1
        raise Abort('unbalanced { at token %d' % idx)

    # ---- edit helpers --------------------------------------------------

    def insert(self, pos, s):
        self.inserts.append((pos, s))

    def replace(self, start, end, s):
        self.replacements.append((start, end, s))

    def crosses_pp(self, a, b):
        """True if any preprocessor directive line lies strictly between
        offsets a and b. A brace pair straddling such a boundary may end up
        unbalanced in some compilation configurations, so such constructs
        are left untouched."""
        if a > b:
            a, b = b, a
        return any(s < b and e > a for (s, e) in self.pp_ranges)

    def line_text(self, ln):
        """Text of line number ln (0-based), without its line ending."""
        ls = self.line_start_of_line(ln)
        le = self.text.find('\n', ls)
        return self.text[ls:le if le >= 0 else len(self.text)]

    def line_start_of_line(self, ln):
        """Start offset of line number ln (0-based)."""
        if ln < 0:
            return 0
        if ln >= len(self.line_offsets):
            return len(self.text)
        return self.line_offsets[ln]

    def close_plan(self, end_idx, kw_idx):
        """Compute (pos, text) for the closing brace after the body statement
        whose token list ends at end_idx (index of the token AFTER the
        statement). kw_idx is the token index of the if/else/while/for/do
        keyword, for indentation.

        Blank lines between the body statement and the next token end up
        AFTER the closing brace, not inside it:
            if (x) {
                return;
            }
            <blank>
        """
        hindent = self.line_indent(self.toks[kw_idx][0])
        if end_idx >= len(self.toks):
            return len(self.text), self.nl + hindent + '}' + self.nl
        nstart = self.toks[end_idx][0]
        prev_tok = self.toks[end_idx - 1]
        if self.line_of(prev_tok[0]) == self.line_of(nstart):
            # Next token is on the same line as the statement end: put the
            # brace right before it (rare; leaves trailing whitespace).
            return nstart, '}' + self.nl + hindent
        # If only blank lines separate the statement from the next token,
        # close the block right after the statement line so the blanks end
        # up outside the braces.
        stmt_line = self.line_of(prev_tok[0])
        next_line = self.line_of(nstart)
        between_blank = next_line > stmt_line + 1 and all(
            self.line_text(ln).strip() == ''
            for ln in range(stmt_line + 1, next_line))
        if between_blank:
            stmt_line_start = self.line_start(prev_tok[0])
            nl_pos = self.text.find('\n', stmt_line_start)
            if nl_pos >= 0:
                return nl_pos + 1, hindent + '}' + self.nl
        return self.line_start(nstart), hindent + '}' + self.nl

    # ---- statement walkers ----------------------------------------------

    def walk(self, i, stop):
        while i < stop:
            w = self.tok_text(i)
            if w in CONTROL:
                i = self.do_cond(i)
            elif w == 'do':
                i = self.do_do(i)
            elif w == 'else':
                i = self.do_else(i)
            else:
                i += 1
        return i

    def do_cond(self, b):
        """b is the index of an if/for/while keyword. Returns the token index
        the main walk should continue from."""
        kw = self.tok_text(b)
        if b + 1 >= len(self.toks) or self.tok_text(b + 1) != '(':
            raise Abort('%s not followed by ( at token %d' % (kw, b))
        close = self.match_paren(b + 1)
        body = close + 1
        if body >= len(self.toks):
            return body
        bw = self.tok_text(body)
        if bw == '{':
            return body  # already braced; main walk continues inside
        if bw == ';':
            self.replace(self.toks[body][0], self.toks[body][1], '{}')
            self.stats['empty'] += 1
            return body + 1
        kw_line = self.line_of(self.toks[b][0])
        body_line = self.line_of(self.toks[body][0])
        hindent = self.line_indent(self.toks[b][0])
        end = self.statement(body)  # nested constructs are handled first
        open_pos = self.toks[close][1]
        if end < len(self.toks) and self.tok_text(end) == 'else':
            # `} else` style: the closing brace joins the else's line
            e_start = self.toks[end][0]
            epos = self.first_nonspace(e_start)
            if self.text[epos:e_start].strip() != '':
                epos = e_start  # else is mid-line; don't jump to line start
            if self.crosses_pp(open_pos, epos):
                self.stats['pp_skip'] += 1
                return end
            if body_line == kw_line:
                self.replace(open_pos, self.toks[body][0],
                             ' {' + self.nl + hindent + self.unit)
                self.stats['same_line'] += 1
            else:
                self.insert(open_pos, ' {')
            self.stats['braced'] += 1
            self.stats['else_join'] += 1
            self.insert(epos, '} ')
            return end
        cpos, ctext = self.close_plan(end, b)
        if self.crosses_pp(open_pos, cpos):
            # brace pair would straddle a preprocessor boundary; leave as-is
            self.stats['pp_skip'] += 1
            return end
        if body_line == kw_line:
            # same-line body: split it onto its own line
            self.replace(self.toks[close][1], self.toks[body][0],
                         ' {' + self.nl + hindent + self.unit)
            self.stats['same_line'] += 1
        else:
            self.insert(open_pos, ' {')
        self.stats['braced'] += 1
        if ctext.startswith('}'):
            self.stats['close_midline'] += 1
        self.insert(cpos, ctext)
        return end

    def do_else(self, b):
        body = b + 1
        if body >= len(self.toks):
            raise Abort('else at end of file, token %d' % b)
        bw = self.tok_text(body)
        if bw == 'if':
            self.stats['else_if'] += 1
            return b + 1  # leave `else if` chains alone
        if bw == '{':
            return body
        if bw == ';':
            self.replace(self.toks[body][0], self.toks[body][1], '{}')
            self.stats['empty'] += 1
            return body + 1
        kw_line = self.line_of(self.toks[b][0])
        body_line = self.line_of(self.toks[body][0])
        hindent = self.line_indent(self.toks[b][0])
        end = self.statement(body)
        open_pos = self.toks[b][1]
        cpos, ctext = self.close_plan(end, b)
        if self.crosses_pp(open_pos, cpos):
            self.stats['pp_skip'] += 1
            return end
        if body_line == kw_line:
            self.replace(open_pos, self.toks[body][0],
                         ' {' + self.nl + hindent + self.unit)
            self.stats['same_line'] += 1
        else:
            self.insert(open_pos, ' {')
        self.stats['braced'] += 1
        if ctext.startswith('}'):
            self.stats['close_midline'] += 1
        self.insert(cpos, ctext)
        return end

    def do_do(self, b):
        body = b + 1
        if body >= len(self.toks):
            raise Abort('do at end of file, token %d' % b)
        bw = self.tok_text(body)
        braced = (bw == '{')
        if braced:
            close_blk = self.match_brace(body)
            self.walk(body + 1, close_blk)
            end = close_blk + 1
        elif bw == ';':
            self.replace(self.toks[body][0], self.toks[body][1], '{}')
            self.stats['empty'] += 1
            end = body + 1
        else:
            end = self.statement(body)
            if end < len(self.toks) and self.tok_text(end) == 'while':
                if end + 1 >= len(self.toks) or self.tok_text(end + 1) != '(':
                    raise Abort('do-while: while not followed by ( at token %d' % end)
                close = self.match_paren(end + 1)
                # `} while (cond);` on the while's line
                pos = self.first_nonspace(self.toks[end][0])
                if self.crosses_pp(self.toks[b][1], pos):
                    self.stats['pp_skip'] += 1
                    return close + 1
                self.stats['braced'] += 1
                self.stats['do'] += 1
                self.insert(self.toks[b][1], ' {')
                self.insert(pos, '} ')
                return close + 1
            cpos, ctext = self.close_plan(end, b)
            if self.crosses_pp(self.toks[b][1], cpos):
                self.stats['pp_skip'] += 1
                return end
            self.stats['braced'] += 1
            self.stats['do'] += 1
            self.insert(self.toks[b][1], ' {')
            self.insert(cpos, ctext)
        if end < len(self.toks) and self.tok_text(end) == 'while':
            if end + 1 >= len(self.toks) or self.tok_text(end + 1) != '(':
                raise Abort('do-while: while not followed by ( at token %d' % end)
            close = self.match_paren(end + 1)
            return close + 1
        return end

    def statement(self, b):
        """Process one statement starting at token index b (the body of an
        unbraced control statement). Returns the index of the token AFTER the
        statement. Nested control statements get braced recursively."""
        w = self.tok_text(b)
        if w in CONTROL:
            return self.do_cond(b)
        if w == 'do':
            return self.do_do(b)
        if w == 'else':
            raise Abort('stray else as body at token %d' % b)
        if w in ('try', 'case', 'default'):
            raise Abort('unhandled body starting with %r at token %d' % (w, b))
        base_line = self.line_of(self.toks[b][0])
        depth = 0
        j = b
        n = len(self.toks)
        while j < n:
            if j - b > MAX_BODY_TOKENS:
                raise Abort('body exceeding %d tokens at token %d' % (MAX_BODY_TOKENS, b))
            if self.line_of(self.toks[j][0]) - base_line > MAX_BODY_LINES:
                raise Abort('body exceeding %d lines at token %d' % (MAX_BODY_LINES, b))
            t = self.tok_text(j)
            if t in ('(', '[', '{'):
                depth += 1
            elif t in (')', ']', '}'):
                depth -= 1
                if depth < 0:
                    raise Abort('body hits closing %r at token %d' % (t, j))
            elif t == ';' and depth == 0:
                return j + 1
            elif depth == 0 and j > b and t in ('if', 'else', 'while', 'for', 'do', 'try'):
                raise Abort('keyword %r inside body at token %d (macro without ;?)' % (t, j))
            j += 1
        raise Abort('no ; terminating body at token %d' % b)

    # ---- output ----------------------------------------------------------

    def result(self):
        ops = [('ins', p, p, s) for (p, s) in self.inserts]
        ops += [('rep', a, b, s) for (a, b, s) in self.replacements]
        ops.sort(key=lambda o: o[1])
        out = []
        cur = 0
        text = self.text
        for kind, a, b, s in ops:
            if a < cur:
                raise Abort('overlapping edits at %d' % a)
            out.append(text[cur:a])
            out.append(s)
            cur = b if kind == 'rep' else a
        out.append(text[cur:])
        return ''.join(out)


def join_else_lines(text, nl):
    """Rewrite

        }
        else ...rest...
    (optionally with a bare `{` line after `else`)

    as

        } else ...rest...

    Only touches adjacent lines with matching indentation where the first
    line consists of exactly `}`. Returns (new_text, joined_count)."""
    lines = text.splitlines(keepends=True)
    out = []
    joined = 0
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        if line.strip() == '}' and i + 1 < n:
            indent = line[:len(line) - len(line.lstrip())]
            nxt = lines[i + 1]
            nxt_stripped = nxt.strip()
            nxt_indent = nxt[:len(nxt) - len(nxt.lstrip())]
            m = re.match(r'else\b', nxt_stripped)
            if m and nxt_indent == indent:
                rest = nxt_stripped[m.end():]
                ending = nxt[len(nxt.rstrip('\r\n')):] if nxt.endswith(('\n', '\r')) else ''
                # absorb a following bare `{` line
                if rest.strip() == '' and i + 2 < n and lines[i + 2].strip() == '{':
                    br = lines[i + 2]
                    br_ending = br[len(br.rstrip('\r\n')):] if br.endswith(('\n', '\r')) else ''
                    out.append(indent + '} else {' + (br_ending or nl))
                    joined += 1
                    i += 3
                    continue
                out.append(indent + '} else' + rest + (ending or nl))
                joined += 1
                i += 2
                continue
        out.append(line)
        i += 1
    return ''.join(out), joined


def check(fx, new, empty_count):
    """Every insertion must consist of braces/whitespace only, and every
    replacement may only consume whitespace or a lone `;` and must produce
    braces/whitespace only. Additionally the total `;` count may only drop
    by the number of empty-statement replacements (`;` -> `{}`)."""
    for pos, s in fx.inserts:
        if not re.fullmatch(r'[{}\s]*', s):
            return False
    for a, b, s in fx.replacements:
        if not re.fullmatch(r'[;\s]*', fx.text[a:b]):
            return False
        if not re.fullmatch(r'[{}\s]*', s):
            return False
    return new.count(';') == fx.text.count(';') - empty_count


def main():
    apply = '--apply' in sys.argv
    diff_name = None
    if '--diff' in sys.argv:
        diff_name = sys.argv[sys.argv.index('--diff') + 1]
    limit = None
    if '--limit' in sys.argv:
        limit = sys.argv[sys.argv.index('--limit') + 1]

    files = sorted(glob.glob(os.path.join(SRC_DIR, '*.cpp')) +
                   glob.glob(os.path.join(SRC_DIR, '*.h')))
    tot = {}
    skipped = []
    for path in files:
        base = os.path.basename(path)
        if base in EXCLUDE:
            continue
        if limit and limit not in base:
            continue
        fx = FileFixer(path)
        try:
            fx.walk(0, len(fx.toks))
        except Abort as ex:
            skipped.append('%s: %s' % (base, ex))
            continue
        new = fx.result()
        if not check(fx, new, fx.stats['empty']):
            skipped.append('%s: SELF-CHECK FAILED' % base)
            continue
        pre_join = new
        new, joined = join_else_lines(new, fx.nl)
        if re.sub(r'\s+', '', new) != re.sub(r'\s+', '', pre_join):
            skipped.append('%s: ELSE-JOIN CHECK FAILED' % base)
            continue
        if not fx.inserts and not fx.replacements and joined == 0:
            continue
        fx.stats['joined'] = joined
        tot[base] = fx.stats
        if diff_name and base == diff_name:
            old_lines = fx.text.splitlines(keepends=True)
            new_lines = new.splitlines(keepends=True)
            sys.stdout.writelines(difflib.unified_diff(
                old_lines, new_lines, fromfile='a/' + base, tofile='b/' + base))
            continue
        if apply:
            data = new.encode('utf-8', errors='surrogateescape')
            if fx.bom:
                data = b'\xef\xbb\xbf' + data
            with open(path, 'wb') as f:
                f.write(data)

    grand = {k: 0 for k in ('braced', 'else_if', 'empty', 'same_line',
                            'close_midline', 'do', 'pp_skip', 'else_join',
                            'joined')}
    for base in sorted(tot):
        s = tot[base]
        for k in grand:
            grand[k] += s.get(k, 0)
        print('%-24s braced=%-5d else_if_left=%-4d empty=%-3d same_line=%-3d midline_close=%-3d do=%-3d pp_skip=%-3d else_join=%-4d joined=%d'
              % (base, s['braced'], s['else_if'], s['empty'], s['same_line'],
                 s['close_midline'], s['do'], s['pp_skip'],
                 s['else_join'], s.get('joined', 0)))
    print('-' * 110)
    print('TOTAL braced=%d else_if_left=%d empty=%d same_line=%d midline_close=%d do=%d pp_skip=%d else_join=%d joined=%d  files=%d'
          % (grand['braced'], grand['else_if'], grand['empty'], grand['same_line'],
             grand['close_midline'], grand['do'], grand['pp_skip'],
             grand['else_join'], grand['joined'], len(tot)))
    for s in skipped:
        print('SKIPPED ' + s)
    if not apply:
        print('(dry run; pass --apply to write files)')


if __name__ == '__main__':
    main()
