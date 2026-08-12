r"""
Report migration state for exported ASM/C++ bridge symbols.

Inputs:
  - migration_state/known_mfc.txt: ASM function names we do NOT migrate.

Outputs:
  - migration_state/to_migrate.txt: ASM function names we plan to migrate to C++.
  - migration_state/complete.txt: ASM function names already migrated to C++.

The script reads proc declarations from src/mfc_export.inc, then resolves which .obj
owns each symbol by parsing `dumpbin /symbols` output.
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INC = ROOT / "src" / "mfc_export.inc"
DEFAULT_OBJ_DIR = ROOT / "build" / "a2serv_vsc.dir" / "Debug"
DEFAULT_STATE_DIR = ROOT / "migration_state"
DEFAULT_KNOWN = DEFAULT_STATE_DIR / "known_mfc.txt"
DEFAULT_TO_MIGRATE = DEFAULT_STATE_DIR / "to_migrate.txt"
DEFAULT_COMPLETE = DEFAULT_STATE_DIR / "complete.txt"
DEFAULT_MAIN_ASM = ROOT / "src" / "Main.asm"
DEFAULT_VSWHERE = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe")

DECL_RE = re.compile(
    r"^\s*(extern(?:def)?|public)\s+([^\s:;,]+)(?:\s*:\s*([^\s;]+))?",
    re.IGNORECASE,
)

ASM_NAME_RE = re.compile(r"\?(sub_[0-9A-Fa-f]+|FUN_[0-9A-Fa-f]+)@")
PROC_NEAR_RE = re.compile(r"^\s*([^\s;]+)\s+proc\s+near\b", re.IGNORECASE)
ENDP_RE = re.compile(r"^\s*([^\s;]+)\s+endp\b", re.IGNORECASE)


@dataclass(frozen=True)
class ExportSymbol:
    line_no: int
    keyword: str
    symbol: str
    raw_line: str
    asm_name: Optional[str]


def normalize_asm_name(name: str) -> str:
    s = name.strip()
    if not s:
        return ""

    if s.startswith("?"):
        at = s.find("@")
        if at > 1:
            s = s[1:at]
        else:
            s = s[1:]

    if s.startswith("_"):
        s = s[1:]

    return s.lower()


def normalize_symbol_text(name: str) -> str:
    return name.strip().lower()


def extract_asm_name(symbol: str) -> Optional[str]:
    s = symbol.strip()

    if s.startswith("_sub_"):
        return s[1:]
    if s.startswith("sub_") or s.startswith("FUN_"):
        return s

    m = ASM_NAME_RE.search(s)
    if m:
        return m.group(1)

    return None


def parse_export_proc_symbols(inc_path: Path) -> List[ExportSymbol]:
    results: List[ExportSymbol] = []

    with inc_path.open("r", encoding="utf-8", errors="replace") as f:
        for lineno, line in enumerate(f, 1):
            m = DECL_RE.match(line)
            if not m:
                continue

            keyword = m.group(1).lower()
            symbol = m.group(2).strip()
            symbol_type = (m.group(3) or "").strip().lower()

            # Keep only function declarations that are explicitly typed as proc.
            if symbol_type != "proc":
                continue

            results.append(
                ExportSymbol(
                    line_no=lineno,
                    keyword=keyword,
                    symbol=symbol,
                    raw_line=line.rstrip(),
                    asm_name=extract_asm_name(symbol),
                )
            )

    return results


def load_name_file(path: Path) -> Tuple[Set[str], Dict[str, str]]:
    normalized: Set[str] = set()
    display: Dict[str, str] = {}

    if not path.is_file():
        return normalized, display

    with path.open("r", encoding="utf-8", errors="replace") as f:
        for line in f:
            raw = line.strip()
            if not raw or raw.startswith("#"):
                continue

            entry_text = raw.split("#", 1)[0].strip()
            if not entry_text:
                continue

            # Support tab-suffixed metadata (for example: symbol<TAB>asm_span).
            name = entry_text.split("\t", 1)[0].strip()
            if not name:
                continue

            key = normalize_symbol_text(name)
            if not key:
                continue

            normalized.add(key)
            display.setdefault(key, entry_text)

    return normalized, display


def find_dumpbin_path(cli_dumpbin: Optional[str]) -> Optional[Path]:
    if cli_dumpbin:
        p = Path(cli_dumpbin)
        if p.is_file():
            return p
        return None

    env_path = os.environ.get("DUMPBIN")
    if env_path:
        p = Path(env_path)
        if p.is_file():
            return p

    if DEFAULT_VSWHERE.is_file():
        try:
            proc = subprocess.run(
                [
                    str(DEFAULT_VSWHERE),
                    "-latest",
                    "-products",
                    "*",
                    "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-find",
                    r"VC\Tools\MSVC\**\bin\Hostx64\x64\dumpbin.exe",
                ],
                capture_output=True,
                text=True,
                check=False,
            )
        except OSError:
            return None

        for line in proc.stdout.splitlines():
            candidate = line.strip()
            if candidate and Path(candidate).is_file():
                return Path(candidate)

    return None


def parse_defined_external_symbols(dump_text: str) -> Set[str]:
    symbols: Set[str] = set()

    for line in dump_text.splitlines():
        if "|" not in line:
            continue

        left, right = line.split("|", 1)
        if "External" not in left:
            continue
        if "UNDEF" in left:
            continue

        # dumpbin prints an optional pretty signature after the decorated name:
        #   ?Foo@@... (public: void __thiscall ...)
        # Keep only the first token so it matches names from mfc_export.inc.
        sym = right.strip().split(None, 1)[0]
        if sym:
            symbols.add(sym)

    return symbols


def build_owner_index(dumpbin_path: Path, obj_dir: Path) -> Tuple[Dict[str, Set[str]], List[str]]:
    owner_index: Dict[str, Set[str]] = {}
    errors: List[str] = []

    obj_files = sorted(obj_dir.glob("*.obj"))
    for obj in obj_files:
        try:
            proc = subprocess.run(
                [str(dumpbin_path), "/symbols", str(obj)],
                capture_output=True,
                text=True,
                errors="replace",
                check=False,
            )
        except OSError as ex:
            errors.append(f"{obj.name}: failed to execute dumpbin ({ex})")
            continue

        if proc.returncode != 0:
            errors.append(f"{obj.name}: dumpbin returned {proc.returncode}")

        for sym in parse_defined_external_symbols(proc.stdout):
            owner_index.setdefault(sym, set()).add(obj.name)

    return owner_index, errors


def build_main_asm_span_index(asm_path: Path) -> Dict[str, Tuple[str, int]]:
    """
    Build a map of proc label -> (display_label, line_span), where:
      line_span = endp_line - proc_line
    """
    spans: Dict[str, Tuple[str, int]] = {}
    open_proc_lines: Dict[str, int] = {}
    display_labels: Dict[str, str] = {}

    if not asm_path.is_file():
        return spans

    with asm_path.open("r", encoding="utf-8", errors="replace") as f:
        for lineno, line in enumerate(f, 1):
            proc_match = PROC_NEAR_RE.match(line)
            if proc_match:
                label = proc_match.group(1)
                key = normalize_symbol_text(label)
                open_proc_lines[key] = lineno
                display_labels[key] = label
                continue

            endp_match = ENDP_RE.match(line)
            if endp_match:
                label = endp_match.group(1)
                key = normalize_symbol_text(label)
                start_line = open_proc_lines.get(key)
                if start_line is None:
                    continue

                spans[key] = (display_labels.get(key, label), lineno - start_line)
                del open_proc_lines[key]

    return spans


def resolve_asm_span(entry: ExportSymbol, asm_spans: Dict[str, Tuple[str, int]]) -> Tuple[str, Optional[int]]:
    """
    Resolve best proc label + span for a still-ASM export entry.

    Prefer exact symbol label, then ASM short name.
    """
    candidate_labels: List[str] = [entry.symbol]
    if entry.asm_name:
        candidate_labels.append(entry.asm_name)

    for label in candidate_labels:
        key = normalize_symbol_text(label)
        if key in asm_spans:
            display_label, span = asm_spans[key]
            return display_label, span

    if entry.asm_name:
        return entry.asm_name, None
    return entry.symbol, None


def choose_status_for_entry(owners: Set[str]) -> str:
    if not owners:
        return "missing"

    owner = next(iter(owners)).lower()
    if owner == "main.obj":
        return "asm"

    return "cpp"


def entry_lookup_keys(entry: ExportSymbol) -> Set[str]:
    """
    Build lookup keys for an export symbol.

    - full symbol key keeps exact mangled identity
    - asm-name key keeps compatibility with legacy sub_/FUN_ lists
    """
    keys: Set[str] = {normalize_symbol_text(entry.symbol)}
    if entry.asm_name:
        keys.add(normalize_asm_name(entry.asm_name))
    return keys


def collect_auto_to_migrate(
    export_symbols: List[ExportSymbol],
    owner_index: Dict[str, Set[str]],
    known_set: Set[str],
    asm_spans: Dict[str, Tuple[str, int]],
) -> List[str]:
    """
    Build a to_migrate list from all resolved exported methods still in ASM.

        Rules:
        - include every parsed proc export (legacy sub_/FUN_ names and already-mangled ones)
        - include entries with status == "asm" (still non-migrated)
        - exclude any entry listed in known_mfc.txt
    """
    result: Dict[str, str] = {}

    for e in export_symbols:
        if any(key in known_set for key in entry_lookup_keys(e)):
            continue

        owners = owner_index.get(e.symbol, set())
        status = choose_status_for_entry(owners)
        if status != "asm":
            continue

        key = normalize_symbol_text(e.symbol)
        line_text = e.symbol
        proc_label, span = resolve_asm_span(e, asm_spans)
        if span is not None:
            # Keep exact export symbol in column 1, span in column 2.
            line_text = f"{e.symbol}\t{span}"
        elif proc_label:
            # proc_label currently only helps resolution; keep output stable otherwise.
            line_text = e.symbol

        result.setdefault(key, line_text)

    def sort_key(line: str):
        # Sort by class name (if exists), then by span size ascending.
        span = 0
        if '\t' in line:
            span = int(line.split('\t', 1)[1])

        is_free = True
        class_name = line
        if '@' in line:
            class_name = line.split('@')[1]

            if class_name:
                is_free = False

        return is_free, class_name.lower(), span

    return sorted(result.values(), key=sort_key)


def collect_auto_complete(
    export_symbols: List[ExportSymbol],
    owner_index: Dict[str, Set[str]],
    known_set: Set[str],
) -> List[str]:
    """
    Build complete list from all resolved exported methods owned by C++ .obj files.

    Rules:
    - include every parsed proc export (legacy sub_/FUN_ names and already-mangled ones)
    - include entries with status == "cpp"
    - exclude any entry listed in known_mfc.txt
    """
    result: Dict[str, str] = {}

    for e in export_symbols:
        if any(key in known_set for key in entry_lookup_keys(e)):
            continue

        owners = owner_index.get(e.symbol, set())
        if choose_status_for_entry(owners) != "cpp":
            continue

        result.setdefault(normalize_symbol_text(e.symbol), e.symbol)

    return sorted(result.values(), key=str.lower)


def write_name_list_file(path: Path, header_note: str, asm_names: List[str]) -> None:
    lines = [
        "# Auto-generated by tool/migration_state.py",
        "# One export symbol per line (written exactly as in mfc_export.inc).",
        "# Optional second tab-separated column stores ASM span (endp_line - proc_line).",
        f"# {header_note}",
        "",
    ]
    lines.extend(asm_names)
    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Report migration status from mfc_export.inc + .obj symbol owners")
    parser.add_argument("--inc", default=str(DEFAULT_INC), help="Path to mfc_export.inc")
    parser.add_argument("--obj-dir", default=str(DEFAULT_OBJ_DIR), help="Directory containing built .obj files")
    parser.add_argument("--state-dir", default=str(DEFAULT_STATE_DIR), help="Directory with known_mfc.txt/to_migrate.txt")
    parser.add_argument("--known", default=None, help="Optional override for known_mfc.txt")
    parser.add_argument("--to-migrate", default=None, help="Optional override for to_migrate.txt")
    parser.add_argument("--complete", default=None, help="Optional override for complete.txt")
    parser.add_argument("--asm-file", default=str(DEFAULT_MAIN_ASM), help="Path to Main.asm for proc/endp span lookup")
    parser.add_argument("--dumpbin", default=None, help="Path to dumpbin.exe (optional if auto-discovery works)")
    parser.add_argument(
        "--no-rewrite-to-migrate",
        action="store_true",
        help="Do not regenerate to_migrate.txt from current symbol ownership",
    )
    parser.add_argument(
        "--no-rewrite-complete",
        action="store_true",
        help="Do not regenerate complete.txt from current symbol ownership",
    )
    args = parser.parse_args()

    inc_path = Path(args.inc).resolve()
    obj_dir = Path(args.obj_dir).resolve()
    state_dir = Path(args.state_dir).resolve()
    asm_path = Path(args.asm_file).resolve()
    known_path = Path(args.known).resolve() if args.known else state_dir / "known_mfc.txt"
    to_migrate_path = Path(args.to_migrate).resolve() if args.to_migrate else state_dir / "to_migrate.txt"
    complete_path = Path(args.complete).resolve() if args.complete else state_dir / "complete.txt"

    if not inc_path.is_file():
        print(f"ERROR: mfc_export.inc not found: {inc_path}")
        return 1
    if not obj_dir.is_dir():
        print(f"ERROR: obj directory not found: {obj_dir}")
        return 1

    dumpbin_path = find_dumpbin_path(args.dumpbin)
    if dumpbin_path is None:
        print("ERROR: dumpbin.exe not found.")
        print("       Provide --dumpbin <path> or set DUMPBIN environment variable.")
        return 1

    export_symbols = parse_export_proc_symbols(inc_path)

    known_set, known_display = load_name_file(known_path)
    to_migrate_set, to_migrate_display = load_name_file(to_migrate_path)

    owner_index, dump_errors = build_owner_index(dumpbin_path, obj_dir)
    asm_spans = build_main_asm_span_index(asm_path)

    auto_to_migrate = collect_auto_to_migrate(export_symbols, owner_index, known_set, asm_spans)
    auto_complete = collect_auto_complete(export_symbols, owner_index, known_set)
    if not args.no_rewrite_to_migrate:
        write_name_list_file(
            to_migrate_path,
            "Edit migration_state/known_mfc.txt to exclude symbols you do not plan to migrate.",
            auto_to_migrate,
        )
        to_migrate_set, to_migrate_display = load_name_file(to_migrate_path)
    if not args.no_rewrite_complete:
        write_name_list_file(
            complete_path,
            "Already migrated to C++ (derived from .obj symbol ownership).",
            auto_complete,
        )

    missing_declared: List[ExportSymbol] = []
    for e in export_symbols:
        owners = owner_index.get(e.symbol, set())
        if not owners:
            missing_declared.append(e)

    overlap = sorted(known_set & to_migrate_set)

    migrated: List[Tuple[str, str, str]] = []
    still_asm: List[Tuple[str, ExportSymbol]] = []
    unresolved: List[Tuple[str, str]] = []

    # Classify every parsed proc export (not just legacy sub_/FUN_-named ones), so the
    # totals below always add up to `len(export_symbols)`.
    seen_keys: Set[str] = set()
    for e in export_symbols:
        key = normalize_symbol_text(e.symbol)
        if key in seen_keys:
            continue
        seen_keys.add(key)

        if any(k in known_set for k in entry_lookup_keys(e)):
            continue

        owners = owner_index.get(e.symbol, set())
        status = choose_status_for_entry(owners)

        if status == "cpp":
            owner = next(iter(owners))
            migrated.append((e.symbol, e.symbol, owner))
        elif status == "asm":
            still_asm.append((e.symbol, e))
        else:
            unresolved.append((e.symbol, "declared but not defined in any .obj"))

    migrated.sort(key=lambda x: x[0].lower())
    still_asm.sort(key=lambda x: x[0].lower())
    unresolved.sort(key=lambda x: x[0].lower())

    print(f"Using dumpbin: {dumpbin_path}")
    print(f"Parsed proc exports: {len(export_symbols)}")
    print(f"Known MFC entries: {len(known_set)}")
    print(f"To-migrate entries: {len(to_migrate_set)}")
    print()

    known_excluded = len(seen_keys) - len(migrated) - len(still_asm) - len(unresolved)

    still_asm_total_lines = 0
    still_asm_unresolved_spans = 0
    for _, entry in still_asm:
        _, span = resolve_asm_span(entry, asm_spans)
        if span is None:
            still_asm_unresolved_spans += 1
            continue
        still_asm_total_lines += span

    print("Migration status (all parsed exports)")
    print("=" * 72)
    print(f"Migrated in C++ : {len(migrated)}")
    print(f"Still in ASM    : {len(still_asm)}")
    print(f"Unresolved      : {len(unresolved)}")
    print(f"Known MFC (excl): {known_excluded}")
    span_note = f" ({still_asm_unresolved_spans} without a resolvable span)" if still_asm_unresolved_spans else ""
    print(f"Still-ASM lines : {still_asm_total_lines}{span_note}")

    if unresolved:
        print("\n[UNRESOLVED]")
        for asm_name, reason in unresolved:
            print(f"  {asm_name:24s} -> {reason}")

    if overlap or dump_errors or missing_declared:
        print()
        print("Warnings")
        print("=" * 72)

        if overlap:
            print(f"WARNING: {len(overlap)} entries appear in both known_mfc.txt and to_migrate.txt")
            for key in overlap:
                print(f"  {known_display.get(key, key)}")

        if dump_errors:
            print(f"WARNING: dumpbin reported issues for {len(dump_errors)} object files")
            for msg in dump_errors:
                print(f"  {msg}")

        if missing_declared:
            print(f"WARNING: {len(missing_declared)} proc symbols are declared in mfc_export.inc but not defined in any .obj")
            for e in missing_declared:
                print(f"  line {e.line_no:5d}: {e.symbol}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
