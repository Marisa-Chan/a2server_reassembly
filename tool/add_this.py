#!/usr/bin/env python3
"""Prefix every implicit member access with `this->` (code style rule).

Scans src/*.cpp and src/*.h (excluding the vendored dplay.h / dplobby.h)
and rewrites bare field/method accesses inside member function bodies:

    void VisLabel::SetActiveColor(bool isActive)
    {
        color_sh = p_clrsh_Gold;   ->  this->color_sh = p_clrsh_Gold;
        VMethod9();                ->  this->VMethod9();
    }

How it works
  1. Tokenize every file (same tokenizer approach as add_braces.py:
     comments, string/char literals and preprocessor lines are skipped,
     first preprocessor branch wins).
  2. Build a class table from class/struct bodies at file scope in ALL
     files: field/method names, statics, enum constants, nested type
     names, anonymous-struct members (promoted into the outer class),
     `using X::member` imports, and base class lists.
  3. For every member function body - out-of-class definitions
     (`Cls::Method(...) [: initlist] {...}` in .cpp or .h) and inline
     bodies inside class declarations - rewrite identifier tokens that
     resolve to a member of the class or an ancestor.

Safety (deliberately biased towards NOT rewriting)
  * already qualified (`.x`, `->x`, `Cls::x`) - skipped
  * static members, enum constants, nested types, operators, ctors/dtors
    are not rewritten (`this->static_member` would break constant
    expressions; statics in this codebase are only used as constants)
  * parameters and locals shadowing a member are tracked and skipped, so
    a rewrite never changes meaning
  * static member function bodies are skipped entirely (`this` invalid)
  * goto/case labels are skipped

Every edit is a pure insertion of the text `this->` before a token, and
that is verified after computing the new text.

Usage:
  python tool/add_this.py                    # dry run, per-file stats
  python tool/add_this.py --apply            # rewrite files
  python tool/add_this.py --limit visual     # files matching substring
  python tool/add_this.py --diff visual.cpp  # unified diff instead of writing
"""

import difflib
import glob
import os
import re
import sys

SRC_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'src')
EXCLUDE = {'dplay.h', 'dplobby.h'}  # vendored SDK headers, leave untouched

TYPE_KEYWORDS = {'int', 'char', 'short', 'long', 'unsigned', 'signed',
                 'float', 'double', 'bool', 'void', 'auto', 'wchar_t'}

# Tokens that can never be member names (prevents misparsing).
KEYWORDS = TYPE_KEYWORDS | {
    'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'default',
    'return', 'goto', 'break', 'continue', 'new', 'delete', 'this',
    'sizeof', 'try', 'catch', 'throw', 'static_cast', 'const_cast',
    'reinterpret_cast', 'dynamic_cast', 'operator', 'const', 'static',
    'virtual', 'inline', 'explicit', 'friend', 'typedef', 'using',
    'namespace', 'template', 'enum', 'struct', 'class', 'union',
    'public', 'private', 'protected', 'override', 'noexcept', 'mutable',
    'constexpr', 'nullptr', 'true', 'false', 'asm', 'alignof',
    'typeid', 'decltype', 'extern', 'register', 'volatile',
    'NULL', 'TRUE', 'FALSE', 'final',
}

ACCESS = ('public', 'private', 'protected')
FOLLOW_DECL = (',', ';', '=', '[', ':', '{')


class Abort(Exception):
    pass


def tokenize(text):
    """Return (tokens, pp_ranges): tokens are (start, end) offsets of code
    words/numbers/punctuation; comments, string/char literals and
    preprocessor directives are skipped. First #if/#else branch wins."""
    toks = []
    pp_ranges = []
    cond_stack = []
    n = len(text)
    i = 0
    while i < n:
        c = text[i]
        if c in ' \t\r\n\f\v':
            i += 1
            continue
        if c == '#':
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
        if c.isalpha() or c == '_':
            j = i + 1
            while j < n and (text[j].isalnum() or text[j] == '_'):
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


class ClassInfo:
    def __init__(self, name):
        self.name = name
        self.fields = set()
        self.methods = set()
        self.statics = set()
        self.types = set()
        self.enum_consts = set()
        self.bases = []  # base class simple names


class FileCtx:
    def __init__(self, path):
        self.path = path
        with open(path, 'rb') as f:
            data = f.read()
        self.bom = data.startswith(b'\xef\xbb\xbf')
        if self.bom:
            data = data[3:]
        self.text = data.decode('utf-8', errors='surrogateescape')
        self.toks, self.pp_ranges = tokenize(self.text)
        self.inserts = []       # char offsets where `this->` is inserted
        self.insert_sites = []  # (class name, token idx) for debugging
        self.inline_bodies = []  # (cls, param_open_tok, body_open_tok)

    def t(self, i):
        if i < 0 or i >= len(self.toks):
            return ''
        return self.text[self.toks[i][0]:self.toks[i][1]]

    def result(self):
        out = []
        cur = 0
        for pos in sorted(self.inserts):
            out.append(self.text[cur:pos])
            out.append('this->')
            cur = pos
        out.append(self.text[cur:])
        return ''.join(out)

    def diff(self, new):
        old_lines = self.text.splitlines(keepends=True)
        new_lines = new.splitlines(keepends=True)
        return ''.join(difflib.unified_diff(
            old_lines, new_lines,
            fromfile='a/' + os.path.basename(self.path),
            tofile='b/' + os.path.basename(self.path)))


# ---------------------------------------------------------------------------
# Phase 1: collect class declarations (file scope) from every file.

class ClassCollector:
    """Parses class/struct bodies at file scope, filling the global table."""

    def __init__(self, fctx, table):
        self.f = fctx
        self.t = fctx.t
        self.table = table
        self.n = len(fctx.toks)

    def run(self):
        i = 0
        depth = 0
        while i < self.n:
            w = self.t(i)
            if w == '{':
                depth += 1
            elif w == '}':
                depth -= 1
            elif depth == 0 and w in ('class', 'struct'):
                i = self.class_head(i)
                continue
            i += 1

    def class_head(self, i):
        """i is at 'class'/'struct' at file scope. Returns the token index
        to continue scanning from (past the whole declaration)."""
        j = i + 1
        if j >= self.n or not self.t(j).isidentifier():
            return i + 1
        name = self.t(j)
        k = j + 1
        while self.t(k).isidentifier() and self.t(k) not in (':',):
            # skip storage-class tokens like `final`
            if self.t(k) in ('final',):
                k += 1
            else:
                break
        if self.t(k) == ';':
            return k + 1  # forward declaration
        bases = self.parse_bases(k)
        if self.t(k) == ':':
            while k < self.n and self.t(k) != '{':
                k += 1
        if k >= self.n or self.t(k) != '{':
            return max(k, i + 1)
        cls = ClassInfo(name)
        cls.bases = bases
        self.table[name] = cls
        close = self.class_body(cls, k + 1)
        return close + 1

    def class_body(self, cls, i):
        """Parse a class body starting after '{'. Returns the token index of
        the closing '}'. Records inline method bodies into the file ctx."""
        static_flag = False
        while i < self.n:
            w = self.t(i)
            if w == '}':
                return i
            if w in ACCESS and self.t(i + 1) == ':':
                i += 2
            elif w == ';':
                i += 1
                static_flag = False
            elif w == 'friend':
                i = self.skip_to_semi(i)
                static_flag = False
            elif w == 'template':
                i = self.skip_angle(i + 1)
            elif w == 'enum':
                i = self.do_enum(cls, i)
                static_flag = False
            elif w in ('class', 'struct', 'union'):
                i = self.do_nested(cls, i)
                static_flag = False
            elif w == 'using':
                i = self.do_using(cls, i)
                static_flag = False
            elif w == 'typedef':
                end = self.skip_to_semi(i)
                name = self.last_ident(end)
                if name:
                    cls.types.add(name)
                i = end
                static_flag = False
            elif w == 'static':
                static_flag = True
                i += 1
            else:
                i = self.do_decl(cls, i, static_flag)
                static_flag = False
        return self.n

    # ---- class body pieces ------------------------------------------------

    def do_enum(self, cls, i):
        """enum [class] [Name] [: type] { enumerators } [vars] ;"""
        j = i + 1
        if self.t(j) in ('class', 'struct'):
            j += 1
        if self.t(j).isidentifier() and self.t(j) not in KEYWORDS:
            cls.types.add(self.t(j))
            j += 1
        if self.t(j) == ':':
            while j < self.n and self.t(j) != '{':
                j += 1
        if self.t(j) != '{':
            return j
        j += 1
        depth = 0
        expect_name = True
        while j < self.n:
            w = self.t(j)
            if w == '{':
                depth += 1
            elif w == '}':
                if depth == 0:
                    break
                depth -= 1
            elif w == '}' and depth > 0:
                depth -= 1
            elif depth == 0 and w == ',':
                expect_name = True
            elif depth == 0 and w == '=':
                # skip the initializer up to ',' / '}' at this depth
                d = 0
                j += 1
                while j < self.n:
                    w2 = self.t(j)
                    if w2 in '([{':
                        d += 1
                    elif w2 in ')]}':
                        if d == 0:
                            j -= 1  # re-process the terminator
                            break
                        d -= 1
                    elif d == 0 and w2 == ',':
                        j -= 1
                        break
                    j += 1
            elif depth == 0 and expect_name and w.isidentifier() and \
                    w not in KEYWORDS:
                cls.enum_consts.add(w)
                expect_name = False
            j += 1
        j += 1  # past '}'
        # optional trailing declarators: `enum {A} x;`
        while j < self.n and self.t(j) != ';':
            w = self.t(j)
            if w.isidentifier() and w not in KEYWORDS and \
                    self.t(j + 1) in (',', ';'):
                cls.fields.add(w)
            if w == '{':
                j = self.skip_brace(j)
            j += 1
        return j + 1 if j < self.n else j

    def do_nested(self, cls, i):
        """Nested class/struct/union. Anonymous bodies promote their members
        into the outer class. Returns the index after the declaration."""
        j = i + 1
        anonymous = not (self.t(j).isidentifier() and self.t(j) not in KEYWORDS)
        if anonymous:
            if self.t(j) != '{':
                return j
            close = self.class_body(cls, j + 1)  # members promoted into cls
            k = close + 1
            while k < self.n and self.t(k) != ';':
                w = self.t(k)
                if w.isidentifier() and w not in KEYWORDS and \
                        self.t(k + 1) in (',', ';'):
                    cls.fields.add(w)
                if w == '{':
                    k = self.skip_brace(k)
                k += 1
            return k + 1 if k < self.n else k
        name = self.t(j)
        k = j + 1
        if self.t(k) == ';':
            cls.types.add(name)
            return k + 1
        sub = ClassInfo(name)
        if self.t(k) == ':':
            sub.bases = self.parse_bases(k)
            while k < self.n and self.t(k) != '{':
                k += 1
        if self.t(k) != '{':
            cls.types.add(name)
            return max(k, j + 1)
        self.table[name] = sub
        close = self.class_body(sub, k + 1)
        cls.types.add(name)
        k = close + 1
        while k < self.n and self.t(k) != ';':
            w = self.t(k)
            if w.isidentifier() and w not in KEYWORDS and \
                    self.t(k + 1) in (',', ';'):
                cls.fields.add(w)
            if w == '{':
                k = self.skip_brace(k)
            k += 1
        return k + 1 if k < self.n else k

    def parse_bases(self, i):
        """Parse an inheritance list starting at ':'. Returns base names."""
        if self.t(i) != ':':
            return []
        segs = []
        depth = 0
        j = i + 1
        start = j
        while j < self.n:
            w = self.t(j)
            if w == '{' and depth == 0:
                segs.append((start, j))
                break
            if w in '([':
                depth += 1
            elif w in ')]':
                depth -= 1
            elif w == ',' and depth == 0:
                segs.append((start, j))
                start = j + 1
            j += 1
        bases = []
        for a, b in segs:
            base = None
            m = b
            while m > a:
                w = self.t(m - 1)
                if w == '>':
                    d = 0
                    while m > a:
                        w = self.t(m - 1)
                        if w == '>':
                            d += 1
                        elif w == '<':
                            d -= 1
                            if d == 0:
                                m -= 1
                                break
                        m -= 1
                    continue
                if w.isidentifier() and w not in ACCESS and \
                        w not in ('virtual', 'const', 'volatile',
                                  'class', 'struct', 'final'):
                    base = w
                    break
                m -= 1
            if base:
                bases.append(base)
        return bases

    def do_using(self, cls, i):
        """using Name = type;  -> type member
           using Base::member; -> imported member (recorded as method)"""
        j = i + 1
        if not (self.t(j).isidentifier() and self.t(j) not in KEYWORDS):
            return self.skip_to_semi(i)
        if self.t(j + 1) == '=':
            cls.types.add(self.t(j))
            return self.skip_to_semi(i)
        if self.t(j + 1) == ':':
            end = self.skip_to_semi(i)
            k = end - 1
            while k > j:
                w = self.t(k)
                if w.isidentifier() and w not in KEYWORDS and w != 'operator':
                    cls.methods.add(w)
                    break
                k -= 1
            return end
        return self.skip_to_semi(i)

    def do_decl(self, cls, i, static_flag):
        """One member declaration up to ';' or an inline body '{...}'.
        Returns the index to continue from."""
        j = i
        depth = 0
        first_paren = None
        target = None
        while j < self.n:
            w = self.t(j)
            if w == '(':
                if depth == 0 and first_paren is None:
                    first_paren = j
                depth += 1
            elif w == ')':
                depth -= 1
            elif depth == 0:
                if w in (';', '{', '}'):
                    target = j
                    break
                if w in ('class', 'struct', 'enum', 'using', 'template'):
                    target = j
                    break
            j += 1
        if target is None:
            return self.n
        if first_paren is not None and first_paren < target:
            # method (or function-pointer field)
            name = self.method_name(first_paren)
            if self.t(target) == '{':
                close = self.skip_brace(target)
                if name and name != 'operator':
                    (cls.statics if static_flag else cls.methods).add(name)
                self.f.inline_bodies.append((cls, first_paren, close,
                                             static_flag))
                if self.t(close + 1) == ';':
                    return close + 2
                return close + 1
            if name == 'operator':
                return target + 1
            if name:
                (cls.statics if static_flag else cls.methods).add(name)
            else:
                fp = self.func_ptr_name(first_paren)
                if fp:
                    (cls.statics if static_flag else cls.fields).add(fp)
            return target + 1
        # plain field declaration (angle/paren depth aware so template
        # arguments like CMap<uint32_t, uint32_t, ...> are not misread as
        # field declarators)
        k = i
        ad = 0
        pd2 = 0
        while k < target:
            w = self.t(k)
            if w == '<':
                ad += 1
            elif w == '>':
                ad -= 1
            elif w == '(':
                pd2 += 1
            elif w == ')':
                pd2 -= 1
            if ad == 0 and pd2 == 0 and w.isidentifier() and \
                    w not in KEYWORDS and \
                    self.t(k + 1) in (',', ';', '=', '[', ':', '{') and \
                    not (self.t(k + 1) == ':' and self.t(k + 2) == ':') and \
                    not (k > 0 and ((self.t(k - 1) == ':' and self.t(k - 2) == ':') or
                                    self.t(k - 1) == '.' or
                                    (self.t(k - 1) == '>' and k > 1 and
                                     self.t(k - 2) == '-'))):
                (cls.statics if static_flag else cls.fields).add(w)
            k += 1
        return target + 1

    # ---- token helpers -----------------------------------------------------

    def method_name(self, paren_idx):
        """Identifier immediately before the parameter '(' of a method, or
        None (function pointer / operator / keyword cases)."""
        k = paren_idx - 1
        while k >= 0 and self.t(k) in ('&', '*'):
            k -= 1
        if k < 0:
            return None
        w = self.t(k)
        if not w.isidentifier() or w in KEYWORDS:
            return None
        return w

    def func_ptr_name(self, paren_idx):
        """int (*fp)(...) -> fp"""
        if self.t(paren_idx) == '(' and self.t(paren_idx + 1) == '*':
            w = self.t(paren_idx + 2)
            if w.isidentifier() and w not in KEYWORDS:
                return w
        return None

    def last_ident(self, end_idx):
        """Last identifier token before token index end_idx."""
        k = end_idx - 1
        while k >= 0:
            w = self.t(k)
            if w.isidentifier() and w not in KEYWORDS:
                return w
            k -= 1
        return None

    def skip_to_semi(self, i):
        d = 0
        while i < self.n:
            w = self.t(i)
            if w in '([{':
                d += 1
            elif w in ')]}':
                d -= 1
            elif w == ';' and d == 0:
                return i + 1
            elif w == '}' and d < 0:
                return i
            i += 1
        return i

    def skip_brace(self, i):
        d = 0
        while i < self.n:
            w = self.t(i)
            if w == '{':
                d += 1
            elif w == '}':
                d -= 1
                if d == 0:
                    return i
            i += 1
        return self.n

    def skip_angle(self, i):
        d = 0
        while i < self.n:
            w = self.t(i)
            if w == '<':
                d += 1
            elif w == '>':
                d -= 1
                if d == 0:
                    return i + 1
            i += 1
        return i


# ---------------------------------------------------------------------------
# Phase 2: class table with inherited member resolution

class Table:
    def __init__(self):
        self.classes = {}
        self._cache = {}

    # dict-style facade over .classes
    def __setitem__(self, name, cls):
        self.classes[name] = cls

    def get(self, name):
        return self.classes.get(name)

    def __contains__(self, name):
        return name in self.classes

    def keys(self):
        return self.classes.keys()

    def usable_members(self, name):
        """Fields+methods of `name` and its resolvable ancestors, minus
        statics / types / enum constants / ctor names / class-name clashes."""
        if name in self._cache:
            return self._cache[name]
        result = self._compute(name, set())
        self._cache[name] = result
        return result

    def _compute(self, name, seen):
        cls = self.classes.get(name)
        if cls is None or name in seen:
            return set()
        seen.add(name)
        m = set(cls.fields) | set(cls.methods)
        for b in cls.bases:
            if b != name and b in self.classes:
                m |= self._compute(b, seen)
        m -= cls.statics
        m -= cls.types
        m -= cls.enum_consts
        m.discard(cls.name)
        m.discard('~' + cls.name)
        m.discard('operator')
        m -= set(self.classes.keys())
        return m


# ---------------------------------------------------------------------------
# Phase 3: rewrite member bodies

class Rewriter:
    def __init__(self, fctx, table):
        self.f = fctx
        self.t = fctx.t
        self.n = len(fctx.toks)
        self.table = table
        self.stats = {'defs': 0, 'inlines': 0, 'rewrites': 0,
                      'static_skipped': 0}

    # -- helpers --------------------------------------------------------------

    def is_qualified(self, j):
        """Is the token at j preceded by `.`, `->` or `::`? (these are
        two-token operators in the single-char punctuation stream)"""
        if j == 0:
            return False
        p = self.t(j - 1)
        if p == '.':
            return True
        if p == '>' and j >= 2 and self.t(j - 2) == '-':
            return True
        if p == ':' and j >= 2 and self.t(j - 2) == ':':
            return True
        return False

    def is_decl_position(self, j):
        """Is the identifier at j being DECLARED (introduces a local/param)?"""
        if j == 0:
            return False
        p = self.t(j - 1)
        if p in TYPE_KEYWORDS:
            return True
        if p.isidentifier() and p not in KEYWORDS:
            return True
        if p == '>':
            # template args end: CArray<int> arr;
            # (`->` is excluded because j-2 would be '-', not '<')
            return j >= 2 and self.t(j - 2) == '<'
        if p in ('*', '&'):
            k = j - 1
            while k >= 0 and self.t(k) in ('*', '&'):
                k -= 1
            if k >= 0:
                w = self.t(k)
                if w in TYPE_KEYWORDS or w == '>' or \
                        (w.isidentifier() and w not in KEYWORDS):
                    return True
        return False

    def parse_params(self, open_idx, locals_set):
        """Collect parameter names from the parameter list at '('."""
        j = open_idx
        d = 0
        prev = '('
        while j < self.n:
            w = self.t(j)
            if w == '(':
                d += 1
            elif w == ')':
                d -= 1
                if d == 0:
                    break
            elif d == 1 and w.isidentifier() and w not in KEYWORDS:
                p = prev
                if p in TYPE_KEYWORDS or p in ('*', '&', '<', '>', ',') or \
                        (p.isidentifier() and p not in KEYWORDS):
                    locals_set.add(w)
            prev = w
            j += 1

    # -- body rewriting ---------------------------------------------------------

    def rewrite_body(self, cls, start, end, params):
        """Rewrite implicit member accesses in tokens [start, end] (token
        indices, inclusive). start may cover a ctor initializer list."""
        members = self.table.usable_members(cls.name)
        if not members:
            return
        locals_set = set(params)
        labels = set()
        decl = None  # {'depth': paren depth at entry, 'state': 'decl'|'init'}
        pd = 0
        k = start
        while k <= end:
            w = self.t(k)
            if w == '(':
                pd += 1
            elif w == ')':
                pd -= 1
                if decl and pd < decl['depth']:
                    decl = None
            elif w == ';':
                decl = None
            elif w == '=' and decl and pd == decl['depth']:
                decl['state'] = 'init'
            elif w == ',' and decl and pd == decl['depth'] and \
                    decl['state'] == 'init':
                decl['state'] = 'decl'
            elif w.isidentifier() and w not in KEYWORDS:
                nxt = self.t(k + 1) if k + 1 < self.n else ''
                nxt_is_qual = nxt == ':' and k + 2 < self.n and \
                    self.t(k + 2) == ':'
                prev = self.t(k - 1) if k > 0 else ''
                in_decl = decl is not None and pd == decl['depth']
                # goto-style label definition
                if decl is None and nxt == ':' and not nxt_is_qual and \
                        prev in (';', '{', '}', ''):
                    labels.add(w)
                    k += 1
                    continue
                if w in labels:
                    k += 1
                    continue
                if in_decl and decl['state'] == 'decl' and \
                        nxt in FOLLOW_DECL + ('(',) and not nxt_is_qual:
                    # additional declarator: int a = 0, count = 0;
                    # or ctor-style init: Foo a(x), b(y);
                    locals_set.add(w)
                    k += 1
                    continue
                if w in members and not self.is_qualified(k) and \
                        w not in locals_set:
                    if decl is None and self.is_decl_position(k):
                        locals_set.add(w)
                        decl = {'depth': pd, 'state': 'decl'}
                    else:
                        self.f.inserts.append(self.f.toks[k][0])
                        self.f.insert_sites.append((cls.name, k))
                        self.stats['rewrites'] += 1
                elif w not in members and decl is None and \
                        nxt in (',', ';', '=', '[', ':', '{') and \
                        not nxt_is_qual and self.is_decl_position(k):
                    # declaration of a local whose name is not a member:
                    # still enters decl mode so later declarators of the
                    # same statement are tracked
                    locals_set.add(w)
                    decl = {'depth': pd, 'state': 'decl'}
            k += 1

    # -- out-of-class definition scan -------------------------------------------

    def run_defs(self):
        depth = 0
        i = 0
        while i < self.n:
            w = self.t(i)
            if w == '{':
                depth += 1
                i += 1
                continue
            if w == '}':
                depth -= 1
                i += 1
                continue
            if depth != 0 or w != ':':
                i += 1
                continue
            if not (i + 1 < self.n and self.t(i + 1) == ':'):
                i += 1
                continue  # single ':' (label / ternary / accessor)
            ci = self.class_ident_before(i)
            if ci is None:
                i += 1
                continue
            cls = self.table.classes.get(self.t(ci))
            meth_i = i + 2
            if meth_i < self.n and self.t(meth_i) == '~':
                meth_i += 1
            meth = self.t(meth_i)
            if meth == 'operator':
                j = meth_i + 1
                while j < self.n and self.t(j) != '(':
                    j += 1
            else:
                j = meth_i + 1
            if cls is None or j >= self.n or self.t(j) != '(':
                i += 1
                continue
            close = self.match_paren(j)
            if close is None:
                i += 1
                continue
            k = close + 1
            while k < self.n and self.t(k) in ('const', 'noexcept', 'override'):
                k += 1
            init_start = None
            body_start = None
            if self.t(k) == ':':
                init_start = k
                d = 0
                while k < self.n:
                    w2 = self.t(k)
                    if w2 in '([':
                        d += 1
                    elif w2 in ')]':
                        d -= 1
                    elif w2 == '{' and d == 0:
                        body_start = k
                        break
                    k += 1
            elif self.t(k) == '{':
                body_start = k
            if body_start is None:
                i += 1
                continue
            body_end = self.match_brace(body_start)
            if body_end is None:
                i += 1
                continue
            self.stats['defs'] += 1
            meth_name = '~' + meth if self.t(meth_i) == '~' else meth
            if meth_name.lstrip('~') in cls.statics and not \
                    meth_name.startswith('~'):
                self.stats['static_skipped'] += 1
            else:
                params = set()
                self.parse_params(j, params)
                # NOTE: the ctor initializer list is deliberately NOT
                # rewritten - mem-initializer ids must be plain identifiers
                # (`this->x(...)` is a syntax error there)
                self.rewrite_body(cls, body_start, body_end, params)
            i = body_end + 1

    def class_ident_before(self, colon_idx):
        """Token index of the class identifier before '<...> ::' or ' ::'."""
        k = colon_idx - 1
        if k < 0:
            return None
        if self.t(k) == '>':
            d = 0
            while k >= 0:
                w = self.t(k)
                if w == '>':
                    d += 1
                elif w == '<':
                    d -= 1
                    if d == 0:
                        k -= 1
                        break
                k -= 1
            if k < 0 or not self.t(k).isidentifier():
                return None
            return k
        w = self.t(k)
        if w.isidentifier() and w not in KEYWORDS:
            return k
        return None

    def match_paren(self, i):
        d = 0
        while i < self.n:
            w = self.t(i)
            if w == '(':
                d += 1
            elif w == ')':
                d -= 1
                if d == 0:
                    return i
            i += 1
        return None

    def match_brace(self, i):
        d = 0
        while i < self.n:
            w = self.t(i)
            if w == '{':
                d += 1
            elif w == '}':
                d -= 1
                if d == 0:
                    return i
            i += 1
        return None

    # -- inline bodies -------------------------------------------------------------

    def run_inlines(self):
        for cls, param_open, _body_close, static_flag in self.f.inline_bodies:
            close = self.match_paren(param_open)
            if close is None:
                continue
            k = close + 1
            while k < self.n and self.t(k) in ('const', 'noexcept', 'override'):
                k += 1
            # NOTE: a ctor initializer list (':') is deliberately skipped -
            # mem-initializer ids must be plain identifiers
            if self.t(k) == ':':
                d = 0
                while k < self.n:
                    w2 = self.t(k)
                    if w2 in '([':
                        d += 1
                    elif w2 in ')]':
                        d -= 1
                    elif w2 == '{' and d == 0:
                        break
                    k += 1
            if self.t(k) != '{':
                continue
            body_end = self.match_brace(k)
            if body_end is None:
                continue
            self.stats['inlines'] += 1
            if static_flag:
                self.stats['static_skipped'] += 1
                continue
            params = set()
            self.parse_params(param_open, params)
            self.rewrite_body(cls, k, body_end, params)


# ---------------------------------------------------------------------------

def main():
    apply = '--apply' in sys.argv
    diff_name = None
    if '--diff' in sys.argv:
        diff_name = sys.argv[sys.argv.index('--diff') + 1]
    limit = None
    if '--limit' in sys.argv:
        limit = sys.argv[sys.argv.index('--limit') + 1]

    paths = sorted(glob.glob(os.path.join(SRC_DIR, '*.cpp')) +
                   glob.glob(os.path.join(SRC_DIR, '*.h')))
    paths = [p for p in paths if os.path.basename(p) not in EXCLUDE]

    # Phase 1: build the class table from every file first.
    ctxs = {}
    table = Table()
    for p in paths:
        fx = FileCtx(p)
        ctxs[p] = fx
        ClassCollector(fx, table).run()

    if '--dump' in sys.argv:
        name = sys.argv[sys.argv.index('--dump') + 1]
        cls = table.classes.get(name)
        if cls is None:
            print('class %r not found; have: %s' % (name, ', '.join(sorted(table.classes))))
            return
        print('class %s : %s' % (name, ', '.join(cls.bases)))
        print('  fields:  %s' % ', '.join(sorted(cls.fields)))
        print('  methods: %s' % ', '.join(sorted(cls.methods)))
        print('  statics: %s' % ', '.join(sorted(cls.statics)))
        print('  types:   %s' % ', '.join(sorted(cls.types)))
        print('  enums:   %s' % ', '.join(sorted(cls.enum_consts)))
        print('  usable:  %s' % ', '.join(sorted(table.usable_members(name))))
        return

    # Phase 3: rewrite.
    total = {}
    skipped = []
    for p in paths:
        base = os.path.basename(p)
        if limit and limit not in base:
            continue
        fx = ctxs[p]
        rw = Rewriter(fx, table)
        try:
            rw.run_defs()
            rw.run_inlines()
        except Abort as ex:
            skipped.append('%s: %s' % (base, ex))
            continue
        new = fx.result()
        # self-check: only `this->` insertions were made
        if new.count('this->') != fx.text.count('this->') + len(fx.inserts):
            skipped.append('%s: SELF-CHECK FAILED' % base)
            continue
        if not fx.inserts:
            continue
        total[base] = rw.stats
        if diff_name and base == diff_name:
            sys.stdout.write(fx.diff(new))
            continue
        if apply:
            data = new.encode('utf-8', errors='surrogateescape')
            if fx.bom:
                data = b'\xef\xbb\xbf' + data
            with open(p, 'wb') as f:
                f.write(data)

    grand = 0
    for base in sorted(total):
        s = total[base]
        grand += s['rewrites']
        print('%-28s defs=%-4d inlines=%-3d rewrites=%-5d static_skipped=%d'
              % (base, s['defs'], s['inlines'], s['rewrites'],
                 s['static_skipped']))
    print('-' * 90)
    print('TOTAL rewrites=%d files=%d classes=%d'
          % (grand, len(total), len(table.classes)))
    for s in skipped:
        print('SKIPPED ' + s)
    if not apply:
        print('(dry run; pass --apply to write files)')


if __name__ == '__main__':
    main()
