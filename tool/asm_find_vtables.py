"""
Find vtables of CVisualObject-derived classes in Main.asm and propose mangled
names for their still-unnamed (sub_XXXXXX / unknown_libname_NNN) entries.

Strategy:
  1. Parse migration_state/visual_object_hierarchy.txt (child : parent).
  2. Single pass over Main.asm:
       - candidate vtables: `off_XXXXXX dd offset ...` blocks matching the
         CObject layout fingerprint (GetRuntimeClass/Serialize/AssertValid
         near the top);
       - proc boundaries (incl. SEH function chunks) and, per proc, the first
         `call ??0X@@...` (base-ctor call) plus every `offset off_XXXXXX` ref.
  3. Attribute each candidate vtable to a class:
       - definitive: the FIRST vtable ref of a proc already named
         ??0Class@@ / ??1Class@@ / ??_GClass@@ (ctors/dtors install the own
         vtable before any member vtables);
       - otherwise: candidate classes = unattributed children of the base
         class found via the ctor's first `call ??0Base@@`, narrowed by the
         deepest already-named entry class found inside the vtable;
       - fixpoint: a vtable left with exactly one candidate gets it.
  4. For every unnamed entry, find the owner class (shallowest class whose
     vtable contains that exact entry at that slot; the implementation is
     introduced there and inherited by the rest) and propose a rename:
       slot 1 -> ??_GClass@@UAEPAXI@Z  (scalar deleting destructor)
       else   -> nearest ancestor's named entry at that slot, class swapped
       else   -> TODO VMethod{slot-4}@Class (new virtual, signature needed)
  5. Write the full report to migration_state/vtables_report.txt and print a
     short summary (counts, ambiguous vtables, classes without vtables).

Usage:
    python tool/asm_find_vtables.py [--full]   # --full: dump every slot
"""
import sys
import os
import re
from collections import defaultdict

sys.stdout.reconfigure(encoding='utf-8', errors='replace')

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
ASM_PATH = os.path.join(ROOT_DIR, 'src', 'Main.asm')
HIER_PATH = os.path.join(ROOT_DIR, 'migration_state', 'visual_object_hierarchy.txt')
MFC_INC_PATH = os.path.join(ROOT_DIR, 'src', 'mfc_export.inc')
REPORT_PATH = os.path.join(ROOT_DIR, 'migration_state', 'vtables_report.txt')

ROOT_CLASS = 'CVisualObject'
COBJECT = 'CObject'

# Manually curated vtable address -> class mapping, for classes whose vtable
# cannot be auto-attributed (ctors still in ASM and unnamed, etc.).
# Sourced from `//<hexaddr>` comments above class declarations in visual.h.
# Keys are uppercase hex without the `off_` prefix. Add new ones as found.
KNOWN_ADDRESSES = {
    '609730': 'VisCharSellectStats',
    '609820': 'VisCharSellectList',
    '60C9B0': 'VisInvBase',
    '60D0E8': 'VisInvExtBase',
    '60D1A8': 'VisInvExtType1',
    '60D268': 'VisInvExtType2',
    '60D328': 'VisInvExtType3',
}

# ---------------------------------------------------------------- regexes

PROC_RE       = re.compile(r'^(\S+)\s+proc\s+(?:near|far)\b')
ENDP_RE       = re.compile(r'^\S+\s+endp\b')
CHUNK_ON      = '; START OF FUNCTION CHUNK FOR '
CHUNK_OFF     = '; END OF FUNCTION CHUNK FOR'
DATA_RE       = re.compile(r'^\.(?:data|rdata|idata|bss)\b')
CODE_RE       = re.compile(r'^\.code\b')
OFF_RE        = re.compile(r'\boffset\s+(off_[0-9A-Fa-f]+)\b')
CTOR_CALL_RE  = re.compile(r'\bcall\s+(?:j_)?(\?\?0\w+@@\S+)')
DTOR_CALL_RE  = re.compile(r'\bcall\s+(?:j_)?(\?\?1\w+@@\S+)')
SUB_CALL_RE   = re.compile(r'\bcall\s+((?:sub|loc)_[0-9A-Fa-f]+)\b')
CLAIM_RE      = re.compile(r'^\?\?(?:0|1)(\w+)@@')       # ??0X@@ / ??1X@@
SDTOR_RE      = re.compile(r'^\?\?_[GE](\w+)@@')         # ??_GX@@ / ??_EX@@
VT_START_RE   = re.compile(r'^(off_[0-9A-Fa-f]+)\s+dd\s+offset\s+(\S+)')
VT_ENTRY_RE   = re.compile(r'^\s+dd\s+offset\s+(\S+)')
ENTRY_CLASS_RE = re.compile(r'^\?[^@]*@(\w+)@@')
METHOD_RE     = re.compile(r'^\?([^@]+)@(\w+)@@(.*?)(?:_\d+)?$')
CALLCLASS_RE  = re.compile(r'^\?\?[01](\w+)@@')


class VTable:
    def __init__(self, label, line, entries):
        self.label = label
        self.line = line            # 1-based line of the label
        self.entries = entries      # list of entry symbol names
        self.refs = []              # [(proc_name, line)]
        self.cls = None             # attributed class or None
        self.conf = ''              # attribution confidence/method
        self.bound = None           # deepest scope class among entries
        self.skip = False           # out of scope


# ---------------------------------------------------------------- hierarchy

def load_hierarchy(path):
    parent = {}
    order = []
    migrated = set()
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if line.startswith('MIGRATED'):
                _k, _sep, names = line.partition(':')
                migrated.update(names.split())
                continue
            if ':' not in line:
                continue
            child, par = line.split(':', 1)
            child, par = child.strip(), par.strip()
            parent[child] = par
            order.append(child)
    return parent, order, migrated


# ---------------------------------------------------------------- helpers

def entry_class(e):
    m = ENTRY_CLASS_RE.match(e)
    return m.group(1) if m else None


def is_unnamed(e):
    """Entry needs a rename: sub_/unknown_libname, or a mangled name whose
    class is not part of the hierarchy (IDA FLIRT mislabel)."""
    if e.startswith('sub_') or e.startswith('unknown_libname'):
        return True
    if e.startswith('??'):
        return False
    c = entry_class(e)
    return c is not None and c not in SCOPE_AND_COBJECT


# These get bound to the parsed hierarchy in main().
PARENT = {}
CHILDREN = defaultdict(list)
SCOPE = set()
SCOPE_AND_COBJECT = set()

_depth_cache = {}


def depth(c):
    if c in _depth_cache:
        return _depth_cache[c]
    d, cur, seen = 0, c, set()
    while cur in PARENT and cur not in seen:
        seen.add(cur)
        cur = PARENT[cur]
        d += 1
    _depth_cache[c] = d
    return d


def is_ancestor(a, c):
    cur, seen = c, set()
    while cur in PARENT and cur not in seen:
        seen.add(cur)
        cur = PARENT[cur]
        if cur == a:
            return True
    return False


# ---------------------------------------------------------------- main

def main():
    global PARENT, SCOPE, SCOPE_AND_COBJECT
    full = '--full' in sys.argv

    PARENT, hier_order, MIGRATED = load_hierarchy(HIER_PATH)
    MIGRATED.add(ROOT_CLASS)   # CVisualObject ctor/dtor are in C++ already
    SCOPE = set(PARENT) | {ROOT_CLASS}
    MIGRATED &= SCOPE
    for child, par in PARENT.items():
        if par != ROOT_CLASS and par not in PARENT:
            print(f"WARNING: {child}'s parent {par} not in hierarchy; "
                  f"treating as child of {ROOT_CLASS}")
            PARENT[child] = ROOT_CLASS
        CHILDREN[PARENT[child]].append(child)
    SCOPE = set(PARENT) | {ROOT_CLASS}
    SCOPE_AND_COBJECT = SCOPE | {COBJECT}

    if not os.path.isfile(ASM_PATH):
        print(f"ERROR: File not found: {ASM_PATH}")
        sys.exit(1)

    print(f"Reading {ASM_PATH} ...")
    with open(ASM_PATH, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()
    print(f"  {len(lines):,} lines loaded.")

    # --------------------------------------------------------------
    # Pass 1: procs, vtable refs, ctor calls, candidate vtables
    # --------------------------------------------------------------
    proc_refs = defaultdict(list)       # proc -> [(line, off_label)]
    proc_first_ctor = {}                # proc -> first `call ??0X@@` target
    proc_first_sub = {}                 # proc -> first `call sub_X` target
    proc_last_dtor = {}                 # proc -> last `call ??1X@@` (body only)
    vt_refs = defaultdict(list)         # off_label -> [(proc, line)]
    vtables = {}
    all_labels = set()                  # first token of non-indented lines

    current = None
    in_chunk = False
    in_data = False
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        if DATA_RE.match(line):
            in_data = True
            current = None
        elif CODE_RE.match(line):
            in_data = False
        if not in_data:
            if line.startswith(CHUNK_ON):
                current = line[len(CHUNK_ON):].strip()
                in_chunk = True
            elif line.startswith(CHUNK_OFF):
                current = None
                in_chunk = False
            else:
                pm = PROC_RE.match(line)
                if pm:
                    current = pm.group(1)
                elif ENDP_RE.match(line):
                    current = None
                elif current is not None and not in_chunk:
                    m = OFF_RE.search(line)
                    if m:
                        lbl = m.group(1)
                        vt_refs[lbl].append((current, i + 1))
                        proc_refs[current].append((i + 1, lbl))
                    if current not in proc_first_ctor:
                        cm = CTOR_CALL_RE.search(line)
                        if cm:
                            proc_first_ctor[current] = cm.group(1)
                    dm = DTOR_CALL_RE.search(line)
                    if dm:
                        proc_last_dtor[current] = dm.group(1)
                    if current not in proc_first_sub:
                        sm = SUB_CALL_RE.search(line)
                        if sm:
                            proc_first_sub[current] = sm.group(1)
        if not line.startswith((' ', '\t', ';')):
            tok = line.split(None, 1)
            if tok:
                all_labels.add(tok[0])
        # vtable candidate (scan everywhere; fingerprint excludes jump tables)
        vm = VT_START_RE.match(line)
        if vm:
            entries = [vm.group(2)]
            j = i + 1
            while j < n and VT_ENTRY_RE.match(lines[j]):
                entries.append(VT_ENTRY_RE.match(lines[j]).group(1))
                j += 1
            if is_cobject_vtable(entries):
                vtables[vm.group(1)] = VTable(vm.group(1), i + 1, entries)
        i += 1

    print(f"  {len(vtables)} candidate CObject-layout vtables found.")

    for lbl, vt in vtables.items():
        vt.refs = vt_refs.get(lbl, [])
        # deepest scope class among already-named entries
        best, bd = None, -1
        for e in vt.entries:
            c = entry_class(e)
            if c in SCOPE and depth(c) > bd:
                best, bd = c, depth(c)
        vt.bound = best

    # --------------------------------------------------------------
    # Pass 2: attribution
    # --------------------------------------------------------------
    conflicts = []

    def claim_class(proc):
        m = CLAIM_RE.match(proc) or SDTOR_RE.match(proc)
        return m.group(1) if m and m.group(1) in SCOPE else None

    def is_first_ref(vt, proc):
        pr = proc_refs.get(proc)
        return pr and pr[0][1] == vt.label

    # definitive claims from KNOWN_ADDRESSES (curated, highest priority)
    for addr, cls in KNOWN_ADDRESSES.items():
        vt = vtables.get('off_' + addr)
        if vt is None:
            print(f'WARNING: KNOWN_ADDRESSES {addr} ({cls}) — no vtable '
                  f'found at that address')
            continue
        if cls not in SCOPE:
            print(f'WARNING: KNOWN_ADDRESSES {addr} ({cls}) — class not in '
                  f'hierarchy')
            continue
        vt.cls = cls
        vt.conf = 'KNOWN_ADDRESSES'

    # definitive claims from already-named ctor/dtor procs
    for vt in vtables.values():
        if vt.cls:
            continue
        claims = set()
        for proc, _ln in vt.refs:
            c = claim_class(proc)
            if c and is_first_ref(vt, proc):
                claims.add(c)
        if len(claims) == 1:
            vt.cls = claims.pop()
            vt.conf = 'ctor/dtor proc name'
        elif len(claims) > 1:
            conflicts.append((vt, sorted(claims)))
            vt.skip = True

    def ctor_call_candidates(vt, unattributed):
        """Candidate classes from base-ctor calls found in referencing procs.

        A ctor's first `call ??0X@@` is its direct base ctor (MSVC constructs
        bases before members), so the class is a direct child of X.  If the
        base ctor is itself an unnamed sub_, look one level deeper: that
        sub's first ??0 call is the grandparent, so the class is a
        grandchild-or-deeper of it.
        """
        sets = []
        for proc, _ln in vt.refs:
            if not is_first_ref(vt, proc) or claim_class(proc):
                continue
            t = proc_first_ctor.get(proc)
            dist = 1
            if not t:
                s = proc_first_sub.get(proc)
                if s:
                    t = proc_first_ctor.get(s)
                    dist = 2
            if not t:
                # dtor: the LAST ??1 call in the body is the base-class dtor
                # (members destruct first, base last; SEH chunks excluded)
                t = proc_last_dtor.get(proc)
                dist = 1
            if not t:
                continue
            m = CALLCLASS_RE.match(t)
            if not m or m.group(1) not in SCOPE:
                continue
            anc = m.group(1)
            if dist == 1:
                sets.append(set(live_children(anc)))
            else:
                sets.append({c for c in SCOPE
                             if is_ancestor(anc, c)
                             and depth(c) - depth(anc) >= dist})
        if not sets:
            return None
        base = set.intersection(*sets)
        base -= MIGRATED
        if vt.bound:
            base = {c for c in base if c == vt.bound or is_ancestor(vt.bound, c)}
        return base & unattributed

    # Migrated classes (ctor/dtor in C++) cannot own an ASM vtable: drop
    # them from attribution candidates so sibling counts stay accurate.
    def live_children(cls):
        return [c for c in CHILDREN.get(cls, ()) if c not in MIGRATED]

    # fixpoint: single-candidate and leaf-bound attribution
    claimed = {vt.cls for vt in vtables.values() if vt.cls}
    changed = True
    while changed:
        changed = False
        for vt in vtables.values():
            if vt.cls or vt.skip:
                continue
            unattributed = SCOPE - claimed - MIGRATED
            cand = ctor_call_candidates(vt, unattributed)
            if cand is not None and len(cand) == 1:
                vt.cls = next(iter(cand))
                vt.conf = 'single candidate (base-ctor + entry analysis)'
                claimed.add(vt.cls)
                changed = True
            elif cand is None and vt.bound and not CHILDREN.get(vt.bound) \
                    and vt.bound in unattributed:
                # leaf class is the deepest named entry: must be that class
                vt.cls = vt.bound
                vt.conf = 'deepest entry class (leaf)'
                claimed.add(vt.cls)
                changed = True

    # Note: CVisualObject itself has no ASM vtable — its ctor/dtor are
    # `extern` in mfc_export.inc (already implemented in C++), so no
    # off_XXXXXX table should ever be attributed to the root class.

    # --------------------------------------------------------------
    # Split into scope / out-of-scope
    # --------------------------------------------------------------
    scope_vts = [vt for vt in vtables.values() if vt.cls]
    vtable_of = {vt.cls: vt for vt in scope_vts}
    ambiguous = [vt for vt in vtables.values()
                 if not vt.cls and not vt.skip
                 and (vt.bound or ctor_call_candidates(vt, SCOPE))]
    out_of_scope = len(vtables) - len(scope_vts) - len(ambiguous) - len(conflicts)

    # sanity: two vtables claiming the same class
    dupes = defaultdict(list)
    for vt in scope_vts:
        dupes[vt.cls].append(vt.label)
    dupes = {c: ls for c, ls in dupes.items() if len(ls) > 1}

    # --------------------------------------------------------------
    # Pass 3: per-entry analysis
    # --------------------------------------------------------------
    occ = defaultdict(list)             # (entry, slot) -> [class]
    entry_slots = defaultdict(set)      # entry -> {slots}
    for vt in scope_vts:
        for slot, e in enumerate(vt.entries):
            if is_unnamed(e):
                occ[(e, slot)].append(vt.cls)
                entry_slots[e].add(slot)

    def find_owner(classes):
        """Deepest common ancestor of all classes containing this entry —
        the implementation is introduced there and inherited by the rest.
        The LCA may itself lack an ASM vtable (e.g. CVisualObject)."""
        classes = sorted(set(classes), key=depth)
        if not classes:
            return None

        def chain(c):
            res = []
            while c is not None:
                res.append(c)
                c = PARENT.get(c)
            return res

        common = set(chain(classes[0]))
        for c in classes[1:]:
            common &= set(chain(c))
        return max(common, key=depth) if common else None

    # canonical (name, sig) per slot: usable only when every named scope
    # entry at that slot agrees (branch-specific new virtuals conflict and
    # are excluded).
    slot_name_sets = defaultdict(set)
    for vt in scope_vts:
        for slot, e in enumerate(vt.entries):
            pm = METHOD_RE.match(e)
            if pm and pm.group(2) in SCOPE_AND_COBJECT \
                    and not e.startswith('??'):
                slot_name_sets[slot].add((pm.group(1), pm.group(3)))
    slot_names = {s: next(iter(v)) for s, v in slot_name_sets.items()
                  if len(v) == 1}

    def find_method_name(cls, slot):
        """Nearest ancestor's named entry at this slot, falling back to the
        globally-consistent slot name -> (name, sig) or None."""
        a = PARENT.get(cls)
        while a:
            vt = vtable_of.get(a)
            if vt and slot < len(vt.entries):
                e = vt.entries[slot]
                pm = METHOD_RE.match(e)
                if pm and pm.group(2) in SCOPE_AND_COBJECT \
                        and not e.startswith('??'):
                    return pm.group(1), pm.group(3)
            a = PARENT.get(a)
        return slot_names.get(slot)

    renames = {}        # old -> (new, owner, slot, note)
    reviews = []        # (kind, text)

    for (e, slot), classes in sorted(occ.items(),
                                     key=lambda kv: (kv[0][1], kv[0][0])):
        owner = find_owner(classes)
        if len(entry_slots[e]) > 1:
            reviews.append(('cross-slot',
                            f'{e} appears at slots {sorted(entry_slots[e])} '
                            f'across scope vtables — not a plain virtual?'))
            continue
        if e in renames:
            continue
        if slot == 1:
            new = f'??_G{owner}@@UAEPAXI@Z'
            note = 'scalar deleting dtor'
        else:
            nm = find_method_name(owner, slot)
            if nm:
                new = f'?{nm[0]}@{owner}@@{nm[1]}'
                note = ''
            else:
                continue    # TODO handled per-class below
        if owner not in classes:
            note = (note + '; ' if note else '') + \
                'owner has no ASM vtable (inferred from descendants)'
        if e.startswith('?'):
            note = (note + '; ' if note else '') + f'mislabeled by IDA ({e})'
        renames[e] = (new, owner, slot, note)

    # name-exists guard
    for e, (new, owner, slot, note) in list(renames.items()):
        if new in all_labels:
            renames[e] = (new, owner, slot,
                          (note + '; ' if note else '') +
                          'NAME ALREADY EXISTS — verify before renaming')

    # --------------------------------------------------------------
    # Report
    # --------------------------------------------------------------
    out = []
    w = out.append

    w('CVisualObject-family vtable report')
    w('generated by tool/asm_find_vtables.py')
    w('')
    w('SUMMARY')
    w(f'  classes in hierarchy: {len(PARENT)} (+ root {ROOT_CLASS})')
    w(f'  candidate CObject-layout vtables: {len(vtables)}')
    w(f'  attributed to hierarchy classes: {len(scope_vts)}')
    w(f'  ambiguous (scope-related but unattributed): {len(ambiguous)}')
    w(f'  attribution conflicts: {len(conflicts)}')
    w(f'  out of scope (other CObject classes): {out_of_scope}')

    missing = [c for c in hier_order
               if c not in vtable_of and c not in MIGRATED
               and c not in KNOWN_ADDRESSES.values()]
    migrated_no_vt = [c for c in hier_order
                      if c not in vtable_of and c in MIGRATED]
    # inconsistency checks
    bad_mark = [c for c in MIGRATED if c in vtable_of]
    mfc_externs = set()
    if os.path.isfile(MFC_INC_PATH):
        with open(MFC_INC_PATH, encoding='utf-8', errors='replace') as f:
            for line in f:
                m = re.match(r'extern \s+\?\?[01](\w+)@@', line)
                if m:
                    mfc_externs.add(m.group(1))
    unmarked = [c for c in missing if c in mfc_externs]

    # ---- per-class sections in DFS preorder
    def dfs(c):
        yield c
        for ch in CHILDREN.get(c, ()):
            yield from dfs(ch)

    w('')
    w('=' * 70)
    w('PER-CLASS DETAILS (hierarchy preorder)')
    w('=' * 70)

    todos = []
    for cls in dfs(ROOT_CLASS):
        vt = vtable_of.get(cls)
        if vt is None:
            continue
        par = PARENT.get(cls, '-')
        w('')
        w(f'=== {cls} : {par} — {vt.label} @line {vt.line}, '
          f'{len(vt.entries)} slots [{vt.conf}]')
        ref_desc = []
        for proc, ln in vt.refs:
            mark = ' (first)' if is_first_ref(vt, proc) else ''
            ref_desc.append(f'{proc}:{ln}{mark}')
        w('    refs: ' + (', '.join(ref_desc) if ref_desc else 'none'))

        ok = 0
        for slot, e in enumerate(vt.entries):
            if not is_unnamed(e):
                ok += 1
                if full:
                    w(f'    [{slot:2}] {e}')
                continue
            # unnamed / mislabeled entry
            classes = occ.get((e, slot), [])
            owner = find_owner(classes)
            if len(entry_slots[e]) > 1:
                w(f'    [{slot:2}] {e}  REVIEW (appears at multiple slots)')
            elif owner != cls:
                w(f'    [{slot:2}] {e}  inherited impl, owned by {owner} '
                  f'(renamed there)')
            elif e in renames:
                new, _o, _s, note = renames[e]
                w(f'    [{slot:2}] {e}  RENAME -> {new}'
                  + (f'   ({note})' if note else ''))
            else:
                nm = find_method_name(cls, slot)
                if slot > 1:
                    vname = f'VMethod{slot - 4}'
                    ance = 'override of unnamed ancestor virtual' \
                        if any(vtable_of.get(a) and
                               slot < len(vtable_of[a].entries)
                               for a in iter_ancestors(cls)) \
                        else f'new virtual introduced by {cls}'
                    w(f'    [{slot:2}] {e}  TODO {vname}@{cls} '
                      f'({ance}; signature needed)')
                    todos.append((e, cls, slot, vname, ance))
                else:
                    w(f'    [{slot:2}] {e}  TODO (identify manually)')
                    todos.append((e, cls, slot, None, 'manual'))
        w(f'    ok: {ok}/{len(vt.entries)} slots already named')

    # ---- ambiguous
    if ambiguous:
        w('')
        w('=' * 70)
        w(f'AMBIGUOUS VTABLES ({len(ambiguous)}) — need manual attribution')
        w('=' * 70)
        for vt in sorted(ambiguous, key=lambda v: v.label):
            w('')
            w(f'{vt.label} @line {vt.line}, {len(vt.entries)} slots')
            for proc, ln in vt.refs:
                t = proc_first_ctor.get(proc)
                d = proc_last_dtor.get(proc)
                s = proc_first_sub.get(proc)
                hint = ''
                if t:
                    hint += f', first ctor call: {t}'
                if d:
                    hint += f', last dtor call: {d}'
                if s:
                    hint += f', first sub call: {s}'
                mark = ' (first)' if is_first_ref(vt, proc) else ''
                w(f'    ref {proc}:{ln}{mark}{hint}')
            if vt.bound:
                w(f'    deepest named entry class: {vt.bound}')
            cand = ctor_call_candidates(vt, SCOPE)
            if cand:
                w(f'    candidates: {sorted(cand)}')
            elif cand is not None:
                w('    candidates: NONE — base class named in ctor/dtor '
                  'call has no unattributed children (check hierarchy file)')
            # full slot dump; canonical slot names annotated where known
            for slot, e in enumerate(vt.entries):
                if is_unnamed(e):
                    nm = slot_names.get(slot)
                    w(f'      [{slot:2}] {e}'
                      + (f'   (slot: ?{nm[0]}@?@@{nm[1]})' if nm else ''))
                else:
                    w(f'      [{slot:2}] {e}')

    # ---- conflicts / duplicates / review items
    if conflicts or dupes or reviews:
        w('')
        w('=' * 70)
        w('CONFLICTS / REVIEW')
        w('=' * 70)
        for vt, claims in conflicts:
            w(f'{vt.label} @line {vt.line}: claimed by {claims}')
        for c, ls in sorted(dupes.items()):
            w(f'class {c}: multiple vtables {ls}')
        for kind, text in reviews:
            w(f'REVIEW [{kind}] {text}')

    # ---- missing classes
    w('')
    w('=' * 70)
    w(f'CLASSES WITHOUT VTABLE ({len(missing)} missing, '
      f'{len(migrated_no_vt)} migrated as expected)')
    w('=' * 70)
    if missing:
        w('  MISSING — no ASM vtable found and not marked MIGRATED '
          'or KNOWN_ADDRESSES:')
        for c in missing:
            w(f'  {c}')
    else:
        w('  none')
    known_no_vt = [c for a, c in KNOWN_ADDRESSES.items()
                   if 'off_' + a not in vtables]
    if known_no_vt:
        w('')
        w('  WARNING — KNOWN_ADDRESSES entry has no vtable in ASM '
          '(migrated or wrong address):')
        for c in known_no_vt:
            w(f'  {c}')
    if bad_mark:
        w('')
        w('  WARNING — marked MIGRATED but an ASM vtable was attributed:')
        for c in bad_mark:
            w(f'  {c} ({vtable_of[c].label})')
    if unmarked:
        w('')
        w('  WARNING — no vtable and extern ctor in mfc_export.inc; '
          'add to MIGRATED line:')
        for c in unmarked:
            w(f'  {c}')
    w('')
    w('  migrated (expected, no ASM vtable): '
      + (' '.join(migrated_no_vt) if migrated_no_vt else 'none'))

    # ---- rename plan
    w('')
    w('=' * 70)
    w(f'RENAME PLAN ({len(renames)} symbols)')
    w('=' * 70)
    by_owner = defaultdict(list)
    for e, (new, owner, slot, note) in renames.items():
        by_owner[owner].append((slot, e, new, note))
    for cls in dfs(ROOT_CLASS):
        if cls not in by_owner:
            continue
        w('')
        w(f'# {cls}')
        for slot, e, new, note in sorted(by_owner[cls]):
            w(f'tool/asm_rename.py {e} "{new}"'
              + (f'   # slot {slot}: {note}' if note else f'   # slot {slot}'))

    w('')
    w('=' * 70)
    w('mfc_export.inc ADDITIONS')
    w('=' * 70)
    for cls in dfs(ROOT_CLASS):
        if cls not in by_owner:
            continue
        for slot, e, new, note in sorted(by_owner[cls]):
            w(f'externdef {new} :proc')

    # ---- TODO list
    w('')
    w('=' * 70)
    w(f'TODO — NEW UNNAMED VIRTUALS ({len(todos)}) — need body analysis '
      f'for signature')
    w('=' * 70)
    for e, cls, slot, vname, ance in todos:
        w(f'  {e}  {cls} slot {slot}'
          + (f'  -> ?{vname}@{cls}@@??? ({ance})' if vname else ''))

    with open(REPORT_PATH, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out) + '\n')

    # --------------------------------------------------------------
    # stdout summary
    # --------------------------------------------------------------
    print()
    print(f'Attributed: {len(scope_vts)}/{len(PARENT) + 1} scope classes '
          f'(conflicts: {len(conflicts)}, duplicate claims: {len(dupes)})')
    print(f'Ambiguous vtables: {len(ambiguous)}')
    print(f'Classes without vtable: {len(missing)}')
    print(f'Renames proposed: {len(renames)} '
          f'({sum(1 for v in renames.values() if "NAME ALREADY" in v[3])} name clashes)')
    print(f'New unnamed virtuals (need signatures): {len(todos)}')
    if ambiguous:
        print()
        print('Ambiguous: ' + ', '.join(vt.label for vt in ambiguous))
    if missing:
        print('No vtable: ' + ', '.join(missing))
    print()
    print(f'Full report: {REPORT_PATH}')


def iter_ancestors(cls):
    a = PARENT.get(cls)
    while a:
        yield a
        a = PARENT.get(a)


def is_cobject_vtable(entries):
    if len(entries) < 6:
        return False
    head = entries[:6]
    joined = '\n'.join(head)
    if 'Serialize@CObject@@' in joined and 'AssertValid@CObject@@' in joined:
        return True
    if entries[0].startswith('?GetRuntimeClass@') and \
            ('AssertValid@CObject@@' in joined or 'Dump@' in joined):
        return True
    return False


if __name__ == '__main__':
    main()
