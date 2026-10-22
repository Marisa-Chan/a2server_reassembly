"""Ghidra operations shared by tool/ghidra_daemon.py (standalone pyghidra) and
tool/ghidra_gui_server.py (script run inside the Ghidra GUI).

Every op is a plain function `op_xxx(ctx, **args) -> str`. `ctx` is an OpContext
wrapping the open Program. All mutating ops run inside a transaction.
"""
import io
import json
import re
import sys
import threading
import time
import traceback
from http.server import BaseHTTPRequestHandler, HTTPServer

DEFAULT_PORT = 18812

# C++ spellings used in our headers -> Ghidra data type names.
TYPE_ALIASES = {
    "int8_t": "char", "uint8_t": "uchar", "int16_t": "short", "uint16_t": "ushort",
    "int32_t": "int", "uint32_t": "uint", "int64_t": "longlong", "uint64_t": "ulonglong",
    "unsigned char": "uchar", "unsigned short": "ushort", "unsigned int": "uint",
    "unsigned long": "ulong", "BOOL": "int", "BYTE": "uchar", "WORD": "ushort", "DWORD": "uint",
    "size_t": "uint", "void*": "void *",
}
GAME_CATEGORY = "/Game"


class OpError(Exception):
    pass


class OpContext:
    def __init__(self, program, autosave=True):
        self.program = program
        self.autosave = autosave
        self.stop = False
        self.tx_depth = 0
        self.name = None      # set by ProgramRegistry ("server"/"client"); None = standalone ctx
        self.registry = None  # owning ProgramRegistry, when this ctx is served by one
        self._decomp = None
        from ghidra.util.task import ConsoleTaskMonitor
        self.monitor = ConsoleTaskMonitor()

    # -- helpers -----------------------------------------------------------
    @property
    def dtm(self):
        return self.program.getDataTypeManager()

    @property
    def st(self):
        return self.program.getSymbolTable()

    @property
    def fm(self):
        return self.program.getFunctionManager()

    def decompiler(self):
        if self._decomp is None:
            from ghidra.app.decompiler import DecompInterface, DecompileOptions
            di = DecompInterface()
            opts = DecompileOptions()
            opts.grabFromProgram(self.program)
            di.setOptions(opts)
            di.toggleCCode(True)
            di.toggleSyntaxTree(True)
            di.openProgram(self.program)
            self._decomp = di
        return self._decomp

    def dispose(self):
        if self._decomp is not None:
            self._decomp.dispose()
            self._decomp = None

    def transaction(self, name):
        return _Transaction(self, name)

    def addr(self, s):
        """Resolve 'sub_55ECFE' / 'FUN_0055ecfe' / 'off_60AF78' (IDA data name) /
        '0x55ECFE' / '55ECFE' / symbol name."""
        s = str(s).strip()
        m = re.fullmatch(r"(?:sub_|FUN_|loc_|off_|dword_|word_|byte_|qword_|unk_|dbl_|flt_|stru_|0x|0X)?([0-9A-Fa-f]{5,8})", s)
        if m:
            return self.program.getAddressFactory().getDefaultAddressSpace().getAddress(int(m.group(1), 16))
        syms = self.find_symbols(s)
        if not syms:
            raise OpError(f"no address or symbol matches '{s}'")
        addrs = {str(x.getAddress()) for x in syms}
        if len(addrs) > 1:
            raise OpError(f"'{s}' is ambiguous: " + ", ".join(sorted(addrs)))
        return syms[0].getAddress()

    def find_symbols(self, name):
        from ghidra.program.model.symbol import SymbolType
        out = []
        if "::" in name:
            parts = name.split("::")
            ns = self.namespace(parts[:-1], create=False)
            if ns is None:
                return []
            out.extend(self.st.getSymbols(parts[-1], ns))
            return out
        for s in self.st.getSymbols(name):
            out.append(s)
        return out

    def function(self, s, create=False, split=False):
        """Find the function at `s`. create: make one if missing. split: if the address is inside
        another function's body, carve a new function out of it (the header/IDA says one starts here)."""
        a = self.addr(s)
        f = self.fm.getFunctionAt(a)
        if f is None:
            f = self.fm.getFunctionContaining(a)
            if f is not None:
                if create and split:
                    self._split_function(f, a)
                    f = None
                elif not create:
                    raise OpError(f"{a} is inside {f.getName(True)} @ {f.getEntryPoint()} (not an entry point)")
                else:
                    raise OpError(f"{a} is inside {f.getName(True)} @ {f.getEntryPoint()}; pass split=True to carve it out")
        if f is None and create:
            from ghidra.app.cmd.function import CreateFunctionCmd
            cmd = CreateFunctionCmd(a)
            if not cmd.applyTo(self.program, self.monitor):
                raise OpError(f"cannot create function at {a}: {cmd.getStatusMsg()}")
            f = self.fm.getFunctionAt(a)
        if f is None:
            raise OpError(f"no function at {a}")
        return f

    def _split_function(self, container, a):
        """Carve a new function starting at `a` out of `container` (whose body currently
        covers it): free the tail from `a` on, create the function, give back any leftover."""
        from ghidra.app.cmd.function import CreateFunctionCmd
        from ghidra.program.model.address import AddressSet
        if self.program.getListing().getInstructionAt(a) is None:
            cu = self.program.getListing().getCodeUnitContaining(a)
            raise OpError(f"no instruction starts at {a} (inside {cu.getMinAddress()} '{cu}') - wrong address?")
        tail = AddressSet(a, container.getBody().getMaxAddress())
        if tail.contains(container.getEntryPoint()):
            raise OpError(f"splitting at {a} would swallow {container.getName(True)}'s entry point")
        container.setBody(container.getBody().subtract(tail))
        cmd = CreateFunctionCmd(a)
        if not cmd.applyTo(self.program, self.monitor):
            container.setBody(container.getBody().union(tail))
            raise OpError(f"cannot create function at {a}: {cmd.getStatusMsg()}")
        newf = self.fm.getFunctionAt(a)
        leftover = tail.subtract(newf.getBody())
        if not leftover.isEmpty():
            container.setBody(container.getBody().union(leftover))

    def namespace(self, parts, create=True, as_class=True):
        from ghidra.program.model.symbol import SourceType
        from ghidra.app.util import NamespaceUtils
        ns = self.program.getGlobalNamespace()
        for p in parts:
            child = self.st.getNamespace(p, ns)
            if child is None:
                if not create:
                    return None
                child = self.st.createClass(ns, p, SourceType.USER_DEFINED) if as_class else \
                    self.st.createNameSpace(ns, p, SourceType.USER_DEFINED)
            elif as_class and create and child.getSymbol().getSymbolType().toString() == "Namespace":
                child = NamespaceUtils.convertNamespaceToClass(child)
            ns = child
        return ns

    def parse_type(self, text, create_struct=False):
        """Parse a C type string ('Unit *', 'int32_t', 'uint8_t[4]', 'char **') into a Ghidra DataType."""
        from ghidra.program.model.data import (BuiltInDataTypeManager, StructureDataType, CategoryPath,
                                               PointerDataType, ArrayDataType)
        from java.util import ArrayList
        text = re.sub(r"\b(const|volatile|struct|class|enum)\b", " ", text.strip())
        text = re.sub(r"\s+", " ", text).strip()
        m = re.fullmatch(r"([A-Za-z_][\w:]*(?:<.*>)?(?: [A-Za-z_]\w*)*)\s*((?:\s*\*)*)\s*((?:\[\d+\])*)", text)
        if not m:
            raise OpError(f"cannot parse type '{text}'")
        base, stars, arrays = m.group(1).strip(), m.group(2).count("*"), re.findall(r"\[(\d+)\]", m.group(3))
        base = TYPE_ALIASES.get(base, base)
        if base == "void" and stars == 0 and not arrays:
            return self.dtm.getDataType("/void") or BuiltInDataTypeManager.getDataTypeManager().getDataType("/void")
        dt = None
        if base.startswith("/"):
            dt = self.dtm.getDataType(base)
        else:
            hits = ArrayList()
            self.dtm.findDataTypes(base, hits)
            hits = list(hits)
            if len(hits) == 1:
                dt = hits[0]
            elif len(hits) > 1:
                game = [h for h in hits if str(h.getCategoryPath()) == GAME_CATEGORY]
                root = [h for h in hits if str(h.getCategoryPath()) == "/"]
                if len(game) == 1:
                    dt = game[0]
                elif len(root) == 1:
                    dt = root[0]
                else:
                    dt = hits[0]  # same Windows typedef from several headers
            if dt is None:
                dt = BuiltInDataTypeManager.getDataTypeManager().getDataType("/" + base)
            if dt is None and create_struct and stars > 0 and re.fullmatch(r"[A-Za-z_]\w*", base):
                dt = self.dtm.addDataType(StructureDataType(CategoryPath(GAME_CATEGORY), base, 0, self.dtm), None)
        if dt is None:
            raise OpError(f"unknown data type '{base}' (use 'struct create {base} SIZE' first, or --create)")
        for _ in range(stars):
            dt = PointerDataType(dt, self.dtm)
        for n in reversed(arrays):
            dt = ArrayDataType(dt, int(n), dt.getLength(), self.dtm)
        return dt

    def find_struct(self, name):
        from ghidra.program.model.data import Structure
        if name.startswith("/"):
            dt = self.dtm.getDataType(name)
            if dt is None or not isinstance(dt, Structure):
                raise OpError(f"no structure at path '{name}'")
            return dt
        hits = [s for s in self.dtm.getAllStructures() if str(s.getName()) == name]
        if not hits:
            raise OpError(f"no structure named '{name}'")
        if len(hits) > 1:
            game = [s for s in hits if str(s.getCategoryPath()) == GAME_CATEGORY]
            if len(game) == 1:
                return game[0]
            raise OpError(f"'{name}' is ambiguous: " + ", ".join(str(s.getPathName()) for s in hits))
        return hits[0]

    def save(self, why="ghidra_ops"):
        if self.program.isChanged():
            self.program.save(why, self.monitor)
            return True
        return False


class _Transaction:
    def __init__(self, ctx, name):
        self.ctx, self.name, self.tid = ctx, name, None

    def __enter__(self):
        self.tid = self.ctx.program.startTransaction(self.name)
        self.ctx.tx_depth += 1
        return self

    def __exit__(self, et, ev, tb):
        self.ctx.tx_depth -= 1
        self.ctx.program.endTransaction(self.tid, et is None)
        if et is None and self.ctx.autosave and self.ctx.tx_depth == 0:
            self.ctx.save(self.name)
        return False


class ProgramRegistry:
    """Named set of programs behind one daemon (server/client). Programs open lazily
    on first use through `opener(project_path) -> (program, consumer)`."""

    def __init__(self, programs, default, opener, autosave=True, log=print):
        self.paths = dict(programs)     # canonical name -> project path
        self.default = default
        self.opener = opener
        self.autosave = autosave
        self.stop = False
        self.log = log
        self.ctxs = {}                  # canonical name -> OpContext (open programs only)
        self._consumers = {}            # canonical name -> consumer to release (None = not owned)
        self._lock = threading.Lock()

    @classmethod
    def single(cls, ctx):
        """Serve one already-open OpContext (ghidra_gui_server.py: currentProgram)."""
        name = ctx.name or ctx.program.getName()
        reg = cls({name: None}, name, None, autosave=ctx.autosave)
        reg.ctxs = {name: ctx}
        reg._consumers = {name: None}
        ctx.name, ctx.registry = name, reg
        return reg

    def resolve(self, name):
        """Canonical name; accepts aliases: 'client', 'allods2.exe', '/allods2.exe'."""
        if name in self.paths:
            return name
        n = str(name).lstrip("/")
        for canon, path in self.paths.items():
            if n == canon or (path is not None and n == path.lstrip("/")):
                return canon
        raise OpError(f"unknown program '{name}' (valid: {', '.join(self.paths)})")

    def get(self, name=None):
        """(canonical_name, ctx); opens the program on first use."""
        canon = self.resolve(self.default if name is None else name)
        with self._lock:
            ctx = self.ctxs.get(canon)
            if ctx is None:
                if self.opener is None:
                    raise OpError(f"program '{canon}' is not open and this daemon cannot open it")
                t0 = time.time()
                program, consumer = self.opener(self.paths[canon])
                ctx = OpContext(program, autosave=self.autosave)
                ctx.name, ctx.registry = canon, self
                self.ctxs[canon] = ctx
                self._consumers[canon] = consumer
                self.log(f"[ghidra_ops] opened {canon} = {program.getName()} "
                         f"({program.getFunctionManager().getFunctionCount()} functions) in {time.time() - t0:.1f}s")
            return canon, ctx

    def save_all(self, why="ghidra_ops"):
        return [name for name, ctx in self.ctxs.items() if ctx.save(why)]

    def release_all(self):
        for name, ctx in self.ctxs.items():
            consumer = self._consumers.get(name)
            if consumer is not None:
                ctx.program.release(consumer)
                self._consumers[name] = None

    def dispose_all(self):
        for ctx in self.ctxs.values():
            ctx.dispose()


# -- formatting helpers ----------------------------------------------------

def _fmt_func_header(f):
    ns = f.getParentNamespace()
    nsname = "" if ns.isGlobal() else ns.getName(True) + "::"
    return f"{f.getEntryPoint()}  {nsname}{f.getName()}"


def _proto(f):
    return str(f.getPrototypeString(True, True))


def _params_text(f):
    lines = []
    for p in f.getParameters():
        auto = " (auto)" if p.isAutoParameter() else ""
        lines.append(f"  [{p.getOrdinal()}] {p.getDataType().getName()} {p.getName()}{auto}  @ {p.getVariableStorage()}")
    return lines


def _clean_c(text):
    text = str(text).replace("\r\n", "\n")
    return re.sub(r"\n{3,}", "\n\n", text).strip("\n")


# -- read ops --------------------------------------------------------------

def _ret_text(ctx, body):
    """Distinct RET forms in the body, e.g. 'ret 0x8' - the stack bytes the callee pops."""
    rets = []
    for ins in ctx.program.getListing().getInstructions(body, True):
        if str(ins.getMnemonicString()).upper() == "RET":
            n = ins.getScalar(0) if ins.getNumOperands() else None
            t = f"ret {n.getUnsignedValue():#x}" if n is not None else "ret"
            if t not in rets:
                rets.append(t)
    return ", ".join(rets) if rets else "no ret"


def op_info(ctx, target):
    f = ctx.function(target)
    a = f.getEntryPoint()
    body = f.getBody()
    lines = [_fmt_func_header(f), "proto: " + _proto(f),
             f"cc: {f.getCallingConventionName()} ({_ret_text(ctx, body)})   size: {body.getNumAddresses()} bytes   range: {body.getMinAddress()}-{body.getMaxAddress()}",
             f"signature source: {f.getSignatureSource()}"]
    syms = [str(s.getName(True)) + ("" if s.isPrimary() else " (secondary)") for s in ctx.st.getSymbols(a)]
    lines.append("symbols: " + ", ".join(syms))
    lines.append("params:")
    lines.extend(_params_text(f))
    locs = list(f.getLocalVariables())
    if locs:
        lines.append("locals:")
        for v in locs:
            lines.append(f"  {v.getDataType().getName()} {v.getName()}  @ {v.getVariableStorage()}")
    callers = set()
    for r in ctx.program.getReferenceManager().getReferencesTo(a):
        cf = ctx.fm.getFunctionContaining(r.getFromAddress())
        callers.add(cf.getName(True) if cf else str(r.getFromAddress()))
    lines.append(f"callers ({len(callers)}): " + ", ".join(sorted(callers)[:40]) + (" ..." if len(callers) > 40 else ""))
    c = f.getComment()
    if c:
        lines.append("comment: " + str(c))
    return "\n".join(lines)


def op_decompile(ctx, target, timeout=120):
    f = ctx.function(target)
    res = ctx.decompiler().decompileFunction(f, int(timeout), ctx.monitor)
    if not res.decompileCompleted():
        raise OpError("decompilation failed: " + str(res.getErrorMessage()))
    return _clean_c(res.getDecompiledFunction().getC())


def op_context(ctx, target):
    """Everything needed to migrate one function: info, callee prototypes, decompilation."""
    f = ctx.function(target)
    a = str(f.getEntryPoint())
    return "\n".join(["== INFO", op_info(ctx, a), "", "== CALLEES", op_callees(ctx, a), "", "== DECOMPILED", op_decompile(ctx, a)])


def op_disasm(ctx, target):
    f = ctx.function(target)
    listing = ctx.program.getListing()
    out = []
    for ins in listing.getInstructions(f.getBody(), True):
        out.append(f"{ins.getAddress()}  {ins}")
    return "\n".join(out)


def op_xrefs(ctx, target):
    a = ctx.addr(target)
    out = []
    for r in ctx.program.getReferenceManager().getReferencesTo(a):
        cf = ctx.fm.getFunctionContaining(r.getFromAddress())
        who = cf.getName(True) if cf else "?"
        out.append(f"{r.getFromAddress()}  {str(r.getReferenceType()):10} {who}")
    return "\n".join(out) if out else f"no references to {a}"


def op_callees(ctx, target):
    f = ctx.function(target)
    out = []
    for cf in sorted(f.getCalledFunctions(ctx.monitor), key=lambda x: str(x.getEntryPoint())):
        out.append(f"{cf.getEntryPoint()}  {_proto(cf)}")
    return "\n".join(out) if out else "no callees"


def _fmt_data(ctx, data, show_label=True):
    from ghidra.program.model.data import Pointer
    a = data.getAddress()
    label = ""
    if show_label:
        syms = list(dict.fromkeys(str(s.getName(True)) for s in ctx.st.getSymbols(a) if not s.isDynamic()))
        if syms:
            label = "  " + ", ".join(syms)
    val = str(data.getDefaultValueRepresentation())
    if not val.strip():
        val = data.getDataType().getName()
    extra = ""
    if isinstance(data.getDataType(), Pointer):
        tgt = data.getValue()
        if tgt is not None:
            f = ctx.fm.getFunctionAt(tgt)
            if f is not None:
                extra = "  -> " + f.getName(True)
            else:
                ts = ctx.st.getPrimarySymbol(tgt)
                if ts is not None:
                    extra = "  -> " + ts.getName(True)
    return f"{a}  {data.getMnemonicString():10} {val}{label}{extra}"


def op_global(ctx, target, lines=10):
    """Show the global variable / vtable at target: the data item at the address, then the
    next `lines` defined data items. Pointers resolve to function/symbol names; arrays
    expand one element per line."""
    from ghidra.program.model.data import Array
    listing = ctx.program.getListing()
    a = ctx.addr(target)
    data = listing.getDataAt(a)
    note = None
    if data is None:
        data = listing.getDataContaining(a)
        if data is not None:
            note = f"{a} is inside {data.getDataType().getName()} @ {data.getMinAddress()}"
    if data is None:
        data = listing.getDefinedDataAfter(a)
        if data is not None:
            note = f"no data at {a}; next defined data @ {data.getAddress()}"
    if data is None:
        raise OpError(f"no data at or after {a}")
    out = [note] if note else []
    budget = max(0, int(lines)) + 1
    while data is not None and budget > 0:
        if isinstance(data.getDataType(), Array) and data.getNumComponents() > 1:
            out.append(_fmt_data(ctx, data))
            budget -= 1
            n, i = data.getNumComponents(), 0
            while i < n and budget > 0:
                out.append("  " + _fmt_data(ctx, data.getComponent(i), show_label=False))
                budget -= 1
                i += 1
            if i < n:
                out.append(f"  ... {n - i} more elements (raise --lines)")
        else:
            out.append(_fmt_data(ctx, data))
            budget -= 1
        data = listing.getDefinedDataAfter(data.getMaxAddress())
    return "\n".join(out)


def op_find(ctx, pattern, limit=50):
    pat = re.compile(pattern, re.I)
    hits = []
    for f in ctx.fm.getFunctions(True):
        if pat.search(str(f.getName(True))):
            hits.append(f"{f.getEntryPoint()}  {_proto(f)}")
            if len(hits) >= int(limit):
                break
    for s in ctx.st.getSymbolIterator():
        if len(hits) >= int(limit):
            break
        if s.getSymbolType().toString() in ("Label", "Class", "Namespace") and pat.search(str(s.getName(True))):
            hits.append(f"{s.getAddress()}  {s.getSymbolType()} {s.getName(True)}")
    return "\n".join(hits) if hits else "no matches"


def _ns_string(dobj):
    ns = dobj.getNamespace()
    return str(ns.getNamespaceString()) if ns is not None else ""


def op_demangle(ctx, mangled):
    dobj = _demangle(ctx, mangled)
    lines = [str(dobj.getSignature(True)), f"namespace: {_ns_string(dobj) or '(global)'}",
             f"name: {dobj.getName()}", f"cc: {dobj.getCallingConvention()}"]
    return "\n".join(lines)


# -- write ops: functions ----------------------------------------------------

def _demangle(ctx, mangled):
    from ghidra.app.util.demangler.microsoft import MicrosoftDemangler, MicrosoftDemanglerOptions
    from ghidra.app.util.demangler import DemangledFunction
    opts = MicrosoftDemanglerOptions()
    opts.setDemangleOnlyKnownPatterns(False)
    dem = MicrosoftDemangler()
    try:
        dobj = dem.demangle(dem.createMangledContext(mangled, opts, ctx.program, None))
    except Exception as e:
        raise OpError(f"cannot demangle '{mangled}': {e}")
    if dobj is None or not isinstance(dobj, DemangledFunction):
        raise OpError(f"'{mangled}' does not demangle to a function (got {dobj})")
    return dobj


PLACEHOLDER_NAME = re.compile(r"^(arg\d*|a\d*|b|c|d|param_?\d+|unk\w*|unused\w*|p\d+|v\d+|x\d*|y\d*|val\d*|value\d*|n\d*)$", re.I)
SCALAR_NAMES = {"int", "uint", "short", "ushort", "char", "uchar", "byte", "sbyte", "longlong", "ulonglong",
                "undefined4", "undefined2", "undefined1", "undefined", "long", "ulong", "bool", "dword", "word"}


def _snake_case(name):
    """Unit -> unit, VisDropGold -> vis_drop_gold, CVisualObject -> c_visual_object."""
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])", "_", name).lower()


def _is_placeholder(dt, name):
    return str(dt.getName()) in SCALAR_NAMES and (not name or PLACEHOLDER_NAME.match(name))


def _apply_signature(ctx, f, cc, ret_dt, params, names, keep_better=False, cls=None):
    """params: list of DataType (formal, without `this`). names: list of str or None.
    keep_better: where the new param is a placeholder scalar (e.g. `int32_t arg1`) and Ghidra
    already has a user-typed pointer/struct of the same size there, keep Ghidra's type and name.
    cls: for __thiscall members, the class name - `this` becomes an explicit ECX parameter named
    after the class in snake_case (unit, vis_drop_gold), which requires custom storage."""
    from ghidra.program.model.listing import ParameterImpl, ReturnParameterImpl, Function
    from ghidra.program.model.data import PointerDataType
    from ghidra.program.model.symbol import SourceType
    from java.util import ArrayList
    # old params keyed by stack offset, so an explicit `this` (ECX) or a shift never misaligns them
    old_by_stack = {}
    for p in f.getParameters():
        if p.isAutoParameter():
            continue
        vs = p.getVariableStorage()
        if vs.isStackStorage():
            old_by_stack[vs.getStackOffset()] = p
    this_name = _snake_case(cls.split("::")[-1]) if cls and cc == "__thiscall" else None
    plist = ArrayList()
    used = set()
    if this_name:
        this_dt = None
        try:
            this_dt = PointerDataType(ctx.find_struct(cls.split("::")[-1]), ctx.dtm)
        except OpError:
            for p in f.getParameters():  # no struct: reuse an existing register `this` type if any
                if not p.getVariableStorage().isStackStorage():
                    this_dt = p.getDataType()
                    break
            if this_dt is None:
                this_dt = PointerDataType()
        plist.add(ParameterImpl(this_name, this_dt, ctx.program.getRegister("ECX"), ctx.program))
        used.add(this_name)
    off = 4
    for i, dt in enumerate(params):
        name = None
        if names and i < len(names) and names[i]:
            name = names[i]
        old = old_by_stack.get(off) if cc != "__fastcall" else None
        if old is not None and keep_better and _is_placeholder(dt, name) and str(old.getSource()) == "USER_DEFINED":
            odt = old.getDataType()
            if str(odt.getName()) not in SCALAR_NAMES and odt.getLength() == dt.getLength():
                dt, name = odt, str(old.getName())
        if not name and old is not None and str(old.getSource()) != "DEFAULT":
            name = str(old.getName())
        if name in used or name == "this":
            name = None
        name = name or f"param_{i + 1}"
        used.add(name)
        if this_name:
            stack = ctx.program.getAddressFactory().getStackSpace().getAddress(off)
            plist.add(ParameterImpl(name, dt, stack, ctx.program))
        else:
            plist.add(ParameterImpl(name, dt, ctx.program))
        off += max(4, (dt.getLength() + 3) // 4 * 4)
    update = Function.FunctionUpdateType.CUSTOM_STORAGE if this_name else \
        Function.FunctionUpdateType.DYNAMIC_STORAGE_FORMAL_PARAMS
    f.updateFunction(cc, ReturnParameterImpl(ret_dt, ctx.program), plist, update, True, SourceType.USER_DEFINED)


def op_set_mangled(ctx, target, mangled, names=None):
    """Set function name/class/signature from an MSVC-mangled name. names: optional param names."""
    from ghidra.program.model.symbol import SourceType
    dobj = _demangle(ctx, mangled)
    with ctx.transaction("set_mangled " + mangled):
        f = ctx.function(target, create=True)
        a = f.getEntryPoint()
        nsparts = [p for p in _ns_string(dobj).split("::") if p]
        ns = ctx.namespace(nsparts) if nsparts else ctx.program.getGlobalNamespace()
        short = str(dobj.getName())
        for s in list(ctx.st.getSymbols(a)):
            if str(s.getName()) == mangled and s != f.getSymbol():
                s.delete()
        _set_name_ns(ctx, f, short, ns)
        ctx.st.createLabel(a, mangled, ctx.program.getGlobalNamespace(), SourceType.USER_DEFINED)
        cc = str(dobj.getCallingConvention() or "")
        if cc not in [str(c) for c in ctx.fm.getCallingConventionNames()]:
            cc = "__thiscall" if nsparts else "__cdecl"
        ret = dobj.getReturnType().getDataType(ctx.dtm) if dobj.getReturnType() else ctx.parse_type("void")
        params = [p.getType().getDataType(ctx.dtm) for p in dobj.getParameters()]
        if len(params) == 1 and str(params[0].getName()) == "void":
            params = []
        _apply_signature(ctx, f, cc, ret, params, names, cls=nsparts[-1] if nsparts else None)
        if dobj.isVirtual():
            f.setComment(_merge_comment(f.getComment(), "virtual"))
    return op_info(ctx, str(a))


def _merge_comment(old, tag):
    old = str(old) if old else ""
    return old if tag in old else (old + "\n" if old else "") + tag


def _set_name_ns(ctx, f, name, ns):
    """Rename/move a function symbol, removing a same-named plain label at the address first."""
    from ghidra.program.model.symbol import SourceType
    for s in list(ctx.st.getSymbols(f.getEntryPoint())):
        if s != f.getSymbol() and str(s.getName()) == name and s.getParentNamespace() == ns:
            s.delete()
    f.getSymbol().setNameAndNamespace(name, ns, SourceType.USER_DEFINED)


def op_set_proto(ctx, target, ret, params, cc=None, cls=None, name=None, create=False, plain_ns=False,
                 keep_better=False, split=False):
    """Explicit signature. params: list of 'type name' or 'type' strings (without `this`).
    cls: class (or namespace if plain_ns) to put the function in; '' = global."""
    from ghidra.program.model.symbol import SourceType
    with ctx.transaction("set_proto"):
        f = ctx.function(target, create=True, split=split)
        if cls:
            parts = [p for p in cls.split("::") if p]
            if not plain_ns and not create:
                try:
                    ctx.find_struct(parts[-1])
                except OpError:
                    raise OpError(f"no structure named '{parts[-1]}' for class {cls} (create it first, or pass --create)")
            ns = ctx.namespace(parts, as_class=not plain_ns)
            _set_name_ns(ctx, f, name or str(f.getName()), ns)
        elif cls == "":
            _set_name_ns(ctx, f, name or str(f.getName()), ctx.program.getGlobalNamespace())
        elif name:
            _set_name_ns(ctx, f, name, f.getParentNamespace())
        in_class = not f.getParentNamespace().isGlobal() and \
            f.getParentNamespace().getSymbol().getSymbolType().toString() == "Class"
        cc = cc or ("__thiscall" if in_class else str(f.getCallingConventionName()))
        dts, names = [], []
        for p in params or []:
            p = p.strip()
            m = re.fullmatch(r"(.+?[\s\*\]])\s*([A-Za-z_][A-Za-z_0-9]*)", p)
            if m and not re.fullmatch(r"(unsigned|signed|struct|class|const)", m.group(2)):
                dts.append(ctx.parse_type(m.group(1), create))
                names.append(m.group(2))
            else:
                dts.append(ctx.parse_type(p, create))
                names.append(None)
        _apply_signature(ctx, f, cc, ctx.parse_type(ret, create), dts, names, keep_better,
                         cls=cls if cls and not plain_ns else None)
    return op_info(ctx, target)


def op_rename(ctx, target, name, cls=None):
    from ghidra.program.model.symbol import SourceType
    with ctx.transaction("rename"):
        f = ctx.function(target, create=True)
        if cls is not None:
            ns = ctx.namespace([p for p in cls.split("::") if p]) if cls else ctx.program.getGlobalNamespace()
            f.getSymbol().setNameAndNamespace(name, ns, SourceType.USER_DEFINED)
        else:
            f.getSymbol().setName(name, SourceType.USER_DEFINED)
    return _fmt_func_header(f) + "\n" + _proto(f)


def op_set_class(ctx, target, cls):
    return op_rename(ctx, target, str(ctx.function(target).getName()), cls)


def _high_symbol(ctx, f, var):
    res = ctx.decompiler().decompileFunction(f, 120, ctx.monitor)
    if not res.decompileCompleted():
        raise OpError("decompilation failed: " + str(res.getErrorMessage()))
    hf = res.getHighFunction()
    for s in hf.getLocalSymbolMap().getSymbols():
        if str(s.getName()) == var:
            return hf, s
    gs = hf.getGlobalSymbolMap()
    names = [str(s.getName()) for s in hf.getLocalSymbolMap().getSymbols()]
    raise OpError(f"no variable '{var}' in {f.getName()}; have: {', '.join(names)}")


def op_set_var(ctx, target, var, name=None, type=None, create=False):
    """Rename and/or retype a parameter or local (decompiler variable name)."""
    from ghidra.program.model.pcode import HighFunctionDBUtil
    from ghidra.program.model.symbol import SourceType
    if not name and not type:
        raise OpError("give a new name and/or a type")
    with ctx.transaction("set_var"):
        f = ctx.function(target)
        dt = ctx.parse_type(type, create) if type else None
        hf, hs = _high_symbol(ctx, f, var)
        HighFunctionDBUtil.updateDBVariable(hs, name if name else None, dt, SourceType.USER_DEFINED)
    return op_info(ctx, target)


def op_set_comment(ctx, target, text):
    with ctx.transaction("comment"):
        f = ctx.function(target, create=True)
        f.setComment(text)
    return "ok"


# -- write ops: structs ------------------------------------------------------

def op_struct_show(ctx, name):
    s = ctx.find_struct(name)
    lines = [f"{s.getPathName()}  size=0x{s.getLength():X} ({s.getLength()})  packed={s.isPackingEnabled()}"]
    for c in s.getDefinedComponents():
        fname = c.getFieldName() or ""
        cmt = ("  // " + str(c.getComment())) if c.getComment() else ""
        lines.append(f"  0x{c.getOffset():04X}  {c.getDataType().getName():24} {fname}{cmt}")
    return "\n".join(lines)


def op_struct_list(ctx, pattern=".", limit=200):
    pat = re.compile(pattern, re.I)
    out = []
    for s in ctx.dtm.getAllStructures():
        if pat.search(str(s.getName())):
            out.append(f"{s.getPathName()}  size=0x{s.getLength():X}")
            if len(out) >= int(limit):
                break
    return "\n".join(sorted(out)) if out else "no matches"


def op_struct_create(ctx, name, size, category=GAME_CATEGORY):
    from ghidra.program.model.data import StructureDataType, CategoryPath
    with ctx.transaction("struct create"):
        existing = [s for s in ctx.dtm.getAllStructures() if str(s.getName()) == name]
        if existing:
            raise OpError(f"structure already exists: {existing[0].getPathName()}")
        ctx.dtm.addDataType(StructureDataType(CategoryPath(category), name, _int(size), ctx.dtm), None)
    return op_struct_show(ctx, name)


def op_struct_resize(ctx, name, size):
    with ctx.transaction("struct resize"):
        s = ctx.find_struct(name)
        size = _int(size)
        if s.isPackingEnabled():
            raise OpError("cannot resize a packed structure")
        if size < s.getLength():
            comps = [c for c in s.getDefinedComponents() if c.getOffset() + c.getLength() > size]
            if comps:
                raise OpError("defined fields beyond new size: " + ", ".join(str(c.getFieldName()) for c in comps))
        s.setLength(size)
    return op_struct_show(ctx, name)


def op_struct_set_field(ctx, name, offset, type, field=None, comment=None, create=False):
    with ctx.transaction("struct field"):
        s = ctx.find_struct(name)
        off = _int(offset)
        dt = ctx.parse_type(type, create)
        ln = dt.getLength()
        if ln <= 0:
            raise OpError(f"type '{type}' has no fixed size")
        if s.isPackingEnabled():
            raise OpError("structure uses packing; set fields in Ghidra GUI")
        if off + ln > s.getLength():
            s.growStructure(off + ln - s.getLength())
        # clear anything overlapping the new field, then replace
        for c in list(s.getDefinedComponents()):
            if c.getOffset() < off + ln and c.getOffset() + c.getLength() > off:
                s.clearComponent(c.getOrdinal())
        s.replaceAtOffset(off, dt, ln, field, comment)
    return op_struct_show(ctx, name)


def op_struct_clear_field(ctx, name, offset):
    with ctx.transaction("struct clear"):
        s = ctx.find_struct(name)
        c = s.getComponentContaining(_int(offset))
        if c is None:
            raise OpError("no component at that offset")
        s.clearComponent(c.getOrdinal())
    return op_struct_show(ctx, name)


def _undefined(ctx, size):
    from ghidra.program.model.data import Undefined, ArrayDataType
    if size in (1, 2, 4, 8):
        return Undefined.getUndefinedDataType(size)
    return ArrayDataType(Undefined.getUndefinedDataType(1), size, 1, ctx.dtm)


def _is_undefined_dt(dt):
    from ghidra.program.model.data import Undefined, DefaultDataType
    return dt is None or Undefined.isUndefined(dt) or isinstance(dt, DefaultDataType) or \
        str(dt.getName()).startswith("undefined")


VTABLE_CATEGORY = GAME_CATEGORY + "/VTables"


def _resolve_or_fallback(ctx, text, size_hint=4):
    """parse_type, falling back to void* for unknown pointer types and undefinedN otherwise."""
    try:
        return ctx.parse_type(text, create_struct=False)
    except OpError:
        if "*" in text:
            return ctx.parse_type("void *")
        return _undefined(ctx, size_hint)


def op_vtable_apply(ctx, cls, slots):
    """Create/refresh `{cls}_VTable`: one typed __thiscall function pointer per slot.
    slots: [{"name", "ret", "params": ["type name", ...]}] in MSVC vtable order (without `this`).
    Existing non-placeholder slot names in Ghidra survive if the header name is VMethodN."""
    from ghidra.program.model.data import (StructureDataType, CategoryPath, FunctionDefinitionDataType,
                                           ParameterDefinitionImpl, PointerDataType, DataTypeConflictHandler)
    vt_name = f"{cls}_VTable"
    with ctx.transaction("vtable apply " + vt_name):
        fcat = CategoryPath(f"{VTABLE_CATEGORY}/{cls}")
        try:
            s = ctx.find_struct(vt_name)
        except OpError:
            s = ctx.dtm.addDataType(StructureDataType(CategoryPath(VTABLE_CATEGORY), vt_name, 0, ctx.dtm), None)
        this_dt = PointerDataType(ctx.find_struct(cls), ctx.dtm)
        old_names = {c.getOffset(): str(c.getFieldName()) for c in s.getDefinedComponents() if c.getFieldName()}
        s.deleteAll()
        used = set()
        for i, sl in enumerate(slots):
            name = sl["name"]
            prev = old_names.get(i * 4)
            if re.fullmatch(r"VMethod\d+|pure_virtual_\d+", name) and prev and \
                    not re.fullmatch(r"VMethod\d+|pure_virtual_\d+|slot\d+", prev):
                name = prev
            while name in used:
                name += "_"
            used.add(name)
            fd = FunctionDefinitionDataType(fcat, name, ctx.dtm)
            fd.setReturnType(_resolve_or_fallback(ctx, sl.get("ret") or "void"))
            params = [ParameterDefinitionImpl(_snake_case(cls.split("::")[-1]), this_dt, None)]
            for j, p in enumerate(sl.get("params") or []):
                m = re.fullmatch(r"(.+?[\s\*\]])\s*([A-Za-z_]\w*)", p.strip())
                ptype, pname = (m.group(1), m.group(2)) if m else (p, f"param_{j + 1}")
                params.append(ParameterDefinitionImpl(pname, _resolve_or_fallback(ctx, ptype), None))
            fd.setArguments(params)
            try:
                fd.setCallingConvention("__thiscall")
            except Exception:
                pass
            old_fd = ctx.dtm.getDataType(fcat, name)
            if old_fd is not None:
                # param names are not part of datatype equivalence, so REPLACE_HANDLER alone
                # would keep the old definition (e.g. with the old `this` param name)
                ctx.dtm.remove(old_fd, ctx.monitor)
            fd = ctx.dtm.addDataType(fd, DataTypeConflictHandler.REPLACE_HANDLER)
            s.add(PointerDataType(fd, ctx.dtm), 4, name, None)
    return f"{vt_name}: {len(slots)} slots"


def op_struct_apply(ctx, name, size, fields=None, shrink=False, vtable=False):
    """Merge a header layout into a Ghidra structure (used by tool/ghidra_sync_structs.py).
    fields: [{offset,size,type,name,placeholder,comment}] (inherited fields included) or None for
    size-only. vtable: offset 0 becomes `{name}_VTable *vptr` (vtable-apply must have run)."""
    from ghidra.program.model.data import StructureDataType, CategoryPath, PointerDataType
    stats = {"set": 0, "renamed": 0, "kept": 0, "fallback": 0, "skipped": 0}
    with ctx.transaction("struct apply " + name):
        size = _int(size)
        try:
            s = ctx.find_struct(name)
        except OpError:
            s = ctx.dtm.addDataType(StructureDataType(CategoryPath(GAME_CATEGORY), name, size, ctx.dtm), None)
        if s.isPackingEnabled():
            raise OpError(f"{name} uses packing; cannot merge")
        if size > s.getLength():
            s.growStructure(size - s.getLength())
        elif size < s.getLength() and shrink:
            beyond = [c for c in s.getDefinedComponents() if c.getOffset() + c.getLength() > size]
            if beyond:
                raise OpError(f"{name}: Ghidra has fields beyond ASSERT_SIZE 0x{size:X}: " +
                              ", ".join(f"{c.getFieldName()}@0x{c.getOffset():X}" for c in beyond[:5]))
            s.setLength(size)
        if fields is None:
            return f"{name}: size 0x{s.getLength():X}"
        fields = list(fields)
        if vtable:
            fields.append({"offset": 0, "size": 4, "type_dt": PointerDataType(ctx.find_struct(f"{name}_VTable"), ctx.dtm),
                           "name": "vptr", "placeholder": False})
        for fl in sorted(fields, key=lambda x: _int(x["offset"])):
            off, fsize, fname = _int(fl["offset"]), _int(fl["size"]), fl["name"]
            placeholder = bool(fl.get("placeholder"))
            if off + fsize > s.getLength() or fsize <= 0:
                stats["skipped"] += 1
                continue
            if vtable and off == 0 and fsize == 4 and "type_dt" not in fl:
                stats["skipped"] += 1  # header's own vptr placeholder; the typed one above wins
                continue
            existing = s.getComponentContaining(off)
            same_slot = existing is not None and existing.getOffset() == off and existing.getLength() == fsize
            ex_defined = existing is not None and not _is_undefined_dt(existing.getDataType())
            ex_name = str(existing.getFieldName()) if existing is not None and existing.getFieldName() else ""
            ex_real_name = bool(ex_name) and not PLACEHOLDER_FIELD.match(ex_name) and not re.match(r"^field_0x", ex_name)
            ex_comment = str(existing.getComment()) if same_slot and existing.getComment() else ""
            # header comments ("Inherited from X") only fill empty slots or refresh a previous one of ours
            if ex_comment and not ex_comment.startswith("Inherited from "):
                comment = ex_comment
            else:
                comment = fl.get("comment") or ex_comment or None
            if "type_dt" in fl:
                dt = fl["type_dt"]
                ex_real_name = False  # the typed vptr always wins
            else:
                try:
                    dt = ctx.parse_type(fl["type"], create_struct=True)
                    if dt.getLength() != fsize:
                        dt = None
                except OpError:
                    dt = None
            if dt is None and same_slot and ex_defined:
                dt = existing.getDataType()  # Ghidra already has a same-sized type here; trust it
                stats["fallback"] += 1
            elif dt is None and "<" in fl["type"]:
                # e.g. CList<Effect *>: any Ghidra CList<...> of the same size has the same layout;
                # prefer the candidate sharing the most identifiers with the header's template args
                prefix = fl["type"].split("<")[0].strip() + "<"
                idents = set(re.findall(r"[A-Za-z_]\w*", fl["type"].split("<", 1)[1]))
                cands = [x for x in ctx.dtm.getAllStructures() if str(x.getName()).startswith(prefix) and x.getLength() == fsize]
                if cands:
                    dt = max(cands, key=lambda x: len(idents & set(re.findall(r"[A-Za-z_]\w*", str(x.getName())))))
                    stats["fallback"] += 1
            if dt is None:
                dt = _undefined(ctx, fsize)
                stats["fallback"] += 1
            if existing is not None and not same_slot and ex_defined and placeholder:
                stats["skipped"] += 1  # Ghidra has a differently-shaped real field here
                continue
            new_name = ex_name if same_slot and ex_real_name and placeholder else fname
            unchanged = same_slot and ex_defined and (
                (placeholder and ex_real_name) or
                (str(existing.getDataType().getName()) == str(dt.getName()) and ex_name == new_name))
            if unchanged:
                if (comment or None) != (ex_comment or None):
                    existing.setComment(comment)
                stats["kept"] += 1
                continue
            for c in list(s.getDefinedComponents()):
                if c.getOffset() < off + fsize and c.getOffset() + c.getLength() > off:
                    s.clearComponent(c.getOrdinal())
            for c in s.getDefinedComponents():  # avoid DuplicateNameException elsewhere in the struct
                if c.getFieldName() and str(c.getFieldName()) == new_name and c.getOffset() != off:
                    c.setFieldName(f"{new_name}_old_0x{c.getOffset():X}")
            s.replaceAtOffset(off, dt, fsize, new_name, comment)
            stats["renamed" if same_slot else "set"] += 1
    return f"{name}: size 0x{s.getLength():X}  set={stats['set']} updated={stats['renamed']} kept={stats['kept']} fallback_type={stats['fallback']} skipped={stats['skipped']}"


PLACEHOLDER_FIELD = re.compile(r"^(field\d*_?(0x|x)?[0-9a-fA-F]+|unk\w*|unknown\w*|pad\w*|_pad\w*|padding\w*|reserved\w*|gap\w*|dummy\w*)$", re.I)


def op_struct_rename(ctx, name, new_name):
    """Rename a structure; a same-named class namespace is renamed too (merged if the target
    namespace already exists) so functions follow."""
    from ghidra.program.model.symbol import SourceType
    with ctx.transaction("struct rename"):
        s = ctx.find_struct(name)
        g = ctx.program.getGlobalNamespace()
        old_ns = ctx.st.getNamespace(str(s.getName()), g)
        s.setName(new_name)
        if old_ns is not None and old_ns.getSymbol().getSymbolType().toString() == "Class":
            new_ns = ctx.st.getNamespace(new_name, g)
            if new_ns is None:
                old_ns.getSymbol().setName(new_name, SourceType.USER_DEFINED)
            else:
                moved = 0
                for sym in list(ctx.st.getSymbols(old_ns)):
                    sym.setNamespace(new_ns)
                    moved += 1
                old_ns.getSymbol().delete()
                return op_struct_show(ctx, new_name) + f"\n(merged {moved} symbols from class {name} into {new_name})"
    return op_struct_show(ctx, new_name)


# -- misc --------------------------------------------------------------------

def op_batch(ctx, ops, stop_on_error=False):
    """Run many ops, each in its own (atomic) transaction, with a single save at the end.
    ops: [{"op":..., "args":{...}}, ...]. Returns one line per op: 'ok ...' or 'ERR ...'."""
    lines = []
    autosave, ctx.autosave = ctx.autosave, False
    try:
        for item in ops:
            op, args = item.get("op"), item.get("args") or {}
            try:
                res = run_op(ctx, op, args)
                lines.append(f"ok   {op} {json.dumps(args, ensure_ascii=False)[:80]} -> {str(res).splitlines()[0] if res else ''}")
            except Exception as e:
                lines.append(f"ERR  {op} {json.dumps(args, ensure_ascii=False)[:80]} -> {e}")
                if stop_on_error:
                    raise OpError("\n".join(lines))
    finally:
        ctx.autosave = autosave
        if autosave:
            ctx.save("batch")
    return "\n".join(lines)


def op_save(ctx):
    return "saved" if ctx.save("manual save") else "nothing to save"


def op_save_all(ctx):
    """Save every open program, not just the addressed one."""
    reg = getattr(ctx, "registry", None)
    if reg is None:
        return op_save(ctx)
    saved = reg.save_all("manual save")
    return ("saved: " + ", ".join(saved)) if saved else "nothing to save"


def op_undo(ctx):
    p = ctx.program
    if not p.canUndo():
        return "nothing to undo"
    desc = str(p.getUndoName())
    p.undo()
    if ctx.autosave:
        ctx.save("undo")
    return "undone: " + desc


def op_status(ctx):
    p = ctx.program
    df = p.getDomainFile()
    lines = []
    reg = getattr(ctx, "registry", None)
    if reg is not None:
        parts = []
        for name in reg.paths:
            c = reg.ctxs.get(name)
            if c is None:
                parts.append(f"{name}=not open (lazy)")
            else:
                parts.append(f"{name}=open({c.fm.getFunctionCount()} fns"
                             + (", changed" if c.program.isChanged() else "") + ")")
        lines.append(f"daemon programs: {'  '.join(parts)}  default: {reg.default}")
    lines += [f"program: {p.getName()}  path: {df.getPathname()}",
              f"functions: {ctx.fm.getFunctionCount()}  unsaved changes: {p.isChanged()}  autosave: {ctx.autosave}",
              f"versioned: {df.isVersioned()}  checked out: {df.isCheckedOut()}  version: {df.getVersion()}"]
    return "\n".join(lines)


def op_python(ctx, code):
    """Escape hatch: run Python inside the daemon. `program`, `ctx`, `programs` are in scope."""
    buf = io.StringIO()
    old = sys.stdout
    reg = getattr(ctx, "registry", None)
    programs = {n: c.program for n, c in reg.ctxs.items()} if reg is not None else \
        {ctx.name or ctx.program.getName(): ctx.program}
    g = {"program": ctx.program, "currentProgram": ctx.program, "ctx": ctx, "monitor": ctx.monitor,
         "programs": programs}
    with ctx.transaction("python"):
        sys.stdout = buf
        try:
            exec(code, g)
        finally:
            sys.stdout = old
    return buf.getvalue()


def op_shutdown(ctx):
    """Stop the daemon (it saves and releases the project on the way out)."""
    ctx.stop = True
    reg = getattr(ctx, "registry", None)
    if reg is not None:
        reg.stop = True
    return "shutting down"


def op_reload(ctx):
    """Re-execute this file in place so op changes apply without restarting the daemon."""
    with open(__file__, encoding="utf-8") as fh:
        exec(compile(fh.read(), __file__, "exec"), globals())
    reg = getattr(ctx, "registry", None)
    for c in (list(reg.ctxs.values()) if reg is not None else [ctx]):
        c.__class__ = globals()["OpContext"]  # pick up method changes on the live context too
        c.__dict__.setdefault("tx_depth", 0)
        c.__dict__.setdefault("stop", False)
    if reg is not None:
        reg.__class__ = globals()["ProgramRegistry"]
    return f"reloaded, {len(OPS)} ops"


def _int(v):
    return int(str(v), 0)


OPS = {name[3:].replace("_", "-"): fn for name, fn in list(globals().items()) if name.startswith("op_")}


def run_op(ctx, op, args):
    fn = globals()["OPS"].get(op)
    if fn is None:
        raise OpError(f"unknown op '{op}'; known: {', '.join(sorted(OPS))}")
    return fn(ctx, **args)


# -- HTTP server -------------------------------------------------------------

def make_handler(reg):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, fmt, *a):
            pass

        def do_POST(self):
            n = int(self.headers.get("Content-Length", "0"))
            program = None
            try:
                req = json.loads(self.rfile.read(n) or b"{}")
                program = req.get("program")
                name, ctx = reg.get(program)
                result = globals()["run_op"](ctx, req.get("op"), req.get("args") or {})
                body = {"ok": True, "result": result, "program": name}
            except OpError as e:
                body = {"ok": False, "error": str(e), "program": program}
            except Exception as e:  # Java exceptions surface here too
                body = {"ok": False, "error": f"{type(e).__name__}: {e}", "trace": traceback.format_exc()[-2000:],
                        "program": program}
            data = json.dumps(body).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
    return Handler


def serve(ctx_or_reg, port=DEFAULT_PORT, should_stop=None, log=print):
    reg = ctx_or_reg if isinstance(ctx_or_reg, ProgramRegistry) else ProgramRegistry.single(ctx_or_reg)
    srv = HTTPServer(("127.0.0.1", port), make_handler(reg))
    srv.timeout = 0.5
    state = " + ".join(f"{n} ({'open' if n in reg.ctxs else 'lazy'})" for n in reg.paths)
    log(f"[ghidra_ops] serving {state} on http://127.0.0.1:{port}/  (autosave={reg.autosave})  default: {reg.default}")
    try:
        while not reg.stop and not (should_stop and should_stop()):
            srv.handle_request()
    except KeyboardInterrupt:
        pass
    finally:
        srv.server_close()
        reg.dispose_all()
