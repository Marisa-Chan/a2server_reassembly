#!/usr/bin/env python3

import argparse
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple


@dataclass
class FieldDef:
	name: str
	size: Optional[int]
	type_name: Optional[str] = None
	offset: Optional[int] = None


@dataclass
class StructDef:
	name: str
	align: int = 1
	fields: List[FieldDef] = field(default_factory=list)
	size: Optional[int] = None


_STRUCT_START_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s+struc\b", re.IGNORECASE)
_STRUCT_END_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s+ends\b", re.IGNORECASE)
_STRUCT_ALIGN_RE = re.compile(r"align=0x([0-9A-Fa-f]+)")
_FIELD_SCALAR_RE = re.compile(
	r"^\s*([A-Za-z_]\w*)\s+(db|dw|dd|dq)\b(?:\s+(\d+)\s+dup\(\?\))?",
	re.IGNORECASE,
)
_FIELD_STRUCT_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s+<>", re.IGNORECASE)

_VAR_TYPE_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*([A-Za-z_]\w*)\s+ptr\b")
_GLOBAL_STRUCT_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s+<>")
_PROC_RE = re.compile(r"\bproc\b", re.IGNORECASE)
_ENDP_RE = re.compile(r"\bendp\b", re.IGNORECASE)
_VAR_FIELD_RE = re.compile(r"\b([A-Za-z_]\w*)\.([A-Za-z_][\w\.]*)\b")
_BRACKET_CONTEXT_RE = re.compile(
	r"(?P<prefix>\b(?:byte|word|dword|qword)\s+ptr\s*)?\[(?P<inner>[^\]]+)\]",
	re.IGNORECASE,
)


def _align_up(value: int, align: int) -> int:
	if align <= 1:
		return value
	return (value + (align - 1)) & ~(align - 1)


def _scalar_size(token: str) -> int:
	token = token.lower()
	if token == "db":
		return 1
	if token == "dw":
		return 2
	if token == "dd":
		return 4
	if token == "dq":
		return 8
	raise ValueError(f"Unsupported scalar token: {token}")


def parse_structs(inc_path: Path) -> Dict[str, StructDef]:
	structs: Dict[str, StructDef] = {}
	current: Optional[StructDef] = None

	for raw_line in inc_path.read_text(encoding="utf-8", errors="surrogateescape").splitlines():
		line = raw_line.rstrip("\n")
		start_match = _STRUCT_START_RE.match(line)
		if start_match:
			name = start_match.group(1)
			align_match = _STRUCT_ALIGN_RE.search(line)
			align = int(align_match.group(1), 16) if align_match else 1
			current = StructDef(name=name, align=align)
			structs[name] = current
			continue

		if current is None:
			continue

		if _STRUCT_END_RE.match(line):
			current = None
			continue

		field_match = _FIELD_SCALAR_RE.match(line)
		if field_match:
			name, token, count_raw = field_match.groups()
			count = int(count_raw) if count_raw else 1
			size = _scalar_size(token) * count
			current.fields.append(FieldDef(name=name, size=size))
			continue

		field_struct = _FIELD_STRUCT_RE.match(line)
		if field_struct:
			name, type_name = field_struct.groups()
			current.fields.append(FieldDef(name=name, size=None, type_name=type_name))
			continue

	_resolve_struct_layouts(structs)
	return structs


def _resolve_struct_layouts(structs: Dict[str, StructDef]) -> None:
	unresolved = set(structs.keys())
	progress = True
	while progress and unresolved:
		progress = False
		for name in list(unresolved):
			struct_def = structs[name]
			offsets: List[FieldDef] = []
			offset = 0
			can_resolve = True
			for field in struct_def.fields:
				field_size = field.size
				field_align = 1
				field_type = field.type_name

				if field_size is None:
					if field_type and field_type in structs and structs[field_type].size is not None:
						field_size = structs[field_type].size
						field_align = structs[field_type].align
					else:
						can_resolve = False
						break
				else:
					field_align = min(field_size, struct_def.align) if struct_def.align > 1 else 1

				offset = _align_up(offset, min(field_align, struct_def.align))
				offsets.append(FieldDef(
					name=field.name,
					size=field_size,
					type_name=field_type,
					offset=offset,
				))
				offset += field_size

			if not can_resolve:
				continue

			struct_def.fields = offsets
			struct_def.size = _align_up(offset, struct_def.align)
			unresolved.remove(name)
			progress = True


def _format_offset(offset: int) -> str:
	if offset < 10:
		return str(offset)
	hex_value = format(offset, "X")
	if hex_value[0].isalpha():
		hex_value = "0" + hex_value
	return f"{hex_value}h"


def _get_field_info(
	structs: Dict[str, StructDef],
	type_name: str,
	field_chain: List[str],
) -> Optional[Tuple[int, int]]:
	if type_name not in structs:
		return None
	current_type = type_name
	offset = 0
	final_size: Optional[int] = None

	for field_name in field_chain:
		struct_def = structs.get(current_type)
		if struct_def is None or struct_def.fields is None:
			return None
		field_def = next((f for f in struct_def.fields if f.name == field_name), None)
		if field_def is None or field_def.offset is None or field_def.size is None:
			return None
		offset += field_def.offset
		final_size = field_def.size
		if field_def.type_name is None:
			if field_name != field_chain[-1]:
				return None
			break
		current_type = field_def.type_name

	if final_size is None:
		return None
	return offset, final_size


def _size_to_ptr(size: int) -> Optional[str]:
	if size == 1:
		return "byte ptr"
	if size == 2:
		return "word ptr"
	if size == 4:
		return "dword ptr"
	if size == 8:
		return "qword ptr"
	return None


def _type_supports_field_chain(
	structs: Dict[str, StructDef],
	type_name: str,
	field_chain: List[str],
) -> bool:
	return _get_field_info(structs, type_name, field_chain) is not None


def _infer_type_from_field_chain(
	field_chain: List[str],
	scope_types: Dict[str, str],
	global_types: Dict[str, str],
	structs: Dict[str, StructDef],
) -> Optional[str]:
	local_candidates = {
		type_name
		for type_name in scope_types.values()
		if _type_supports_field_chain(structs, type_name, field_chain)
	}
	if len(local_candidates) == 1:
		return next(iter(local_candidates))
	if local_candidates:
		return None

	global_candidates = {
		type_name
		for type_name in global_types.values()
		if _type_supports_field_chain(structs, type_name, field_chain)
	}
	if len(global_candidates) == 1:
		return next(iter(global_candidates))
	return None


def _replace_accesses(
	text: str,
	scope_types: Dict[str, str],
	global_types: Dict[str, str],
	structs: Dict[str, StructDef],
) -> Tuple[str, List[Tuple[str, int]]]:
	replacements: List[Tuple[str, int]] = []

	def replace_var_field(match: re.Match[str]) -> str:
		nonlocal replacements
		var_name = match.group(1)
		field_chain = match.group(2).split(".")
		type_name = scope_types.get(var_name) or global_types.get(var_name)
		if not type_name:
			type_name = _infer_type_from_field_chain(field_chain, scope_types, global_types, structs)
			if not type_name:
				return match.group(0)
		info = _get_field_info(structs, type_name, field_chain)
		if info is None:
			return match.group(0)
		offset, size = info
		original = f"{var_name}.{'.'.join(field_chain)}"
		replacements.append((original, size))
		if offset == 0:
			return var_name
		return f"{var_name}+{_format_offset(offset)}"

	def replace_bracket(match: re.Match[str]) -> str:
		inner = match.group("inner")
		prefix = match.group("prefix") or ""
		local_replacements: List[Tuple[str, int]] = []

		def replace_var_field_local(local_match: re.Match[str]) -> str:
			nonlocal local_replacements
			var_name = local_match.group(1)
			field_chain = local_match.group(2).split(".")
			type_name = scope_types.get(var_name) or global_types.get(var_name)
			if not type_name:
				type_name = _infer_type_from_field_chain(field_chain, scope_types, global_types, structs)
				if not type_name:
					return local_match.group(0)
			info = _get_field_info(structs, type_name, field_chain)
			if info is None:
				return local_match.group(0)
			offset, size = info
			original = f"{var_name}.{'.'.join(field_chain)}"
			replacements.append((original, size))
			local_replacements.append((original, size))
			if offset == 0:
				return var_name
			return f"{var_name}+{_format_offset(offset)}"

		replaced = _VAR_FIELD_RE.sub(replace_var_field_local, inner)
		if not prefix and len(local_replacements) == 1:
			ptr = _size_to_ptr(local_replacements[0][1])
			if ptr:
				prefix = f"{ptr} "
		return f"{prefix}[{replaced}]"

	updated = _BRACKET_CONTEXT_RE.sub(replace_bracket, text)
	updated = _VAR_FIELD_RE.sub(
		lambda m: _replace_non_bracket(m, scope_types, global_types, structs, replacements),
		updated,
	)
	return updated, replacements


def _replace_non_bracket(
	match: re.Match[str],
	scope_types: Dict[str, str],
	global_types: Dict[str, str],
	structs: Dict[str, StructDef],
	replacements: List[Tuple[str, int]],
) -> str:
	prior_text = match.string[:match.start()]
	if re.search(r"\b(?:byte|word|dword|qword)\s+ptr\s*$", prior_text, re.IGNORECASE):
		return match.group(0)
	var_name = match.group(1)
	field_chain = match.group(2).split(".")
	type_name = scope_types.get(var_name) or global_types.get(var_name)
	if not type_name:
		type_name = _infer_type_from_field_chain(field_chain, scope_types, global_types, structs)
		if not type_name:
			return match.group(0)
	info = _get_field_info(structs, type_name, field_chain)
	if info is None:
		return match.group(0)
	offset, size = info
	ptr = _size_to_ptr(size)
	if not ptr:
		return match.group(0)
	original = f"{var_name}.{'.'.join(field_chain)}"
	replacements.append((original, size))
	if offset == 0:
		return f"{ptr} [{var_name}]"
	return f"{ptr} [{var_name}+{_format_offset(offset)}]"


def _format_replacement_note(replacements: List[Tuple[str, int]]) -> str:
	seen: List[str] = []
	for item in replacements:
		if item[0] not in seen:
			seen.append(item[0])
	if len(seen) == 1:
		return seen[0]
	return ", ".join(seen)


def _parse_global_types(lines: List[str], structs: Dict[str, StructDef]) -> Dict[str, str]:
	globals_map: Dict[str, str] = {}
	for line in lines:
		line_body = line.split(";", 1)[0]
		match = _GLOBAL_STRUCT_RE.match(line_body)
		if not match:
			continue
		var_name, type_name = match.groups()
		if type_name in structs:
			globals_map[var_name] = type_name
	return globals_map


def replace_offsets(asm_path: Path, inc_path: Path, output_path: Optional[Path], in_place: bool) -> int:
	structs = parse_structs(inc_path)
	scope_types: Dict[str, str] = {}
	total_replacements = 0

	lines = asm_path.read_text(encoding="utf-8", errors="surrogateescape").splitlines(keepends=True)
	global_types = _parse_global_types(lines, structs)
	output_lines: List[str] = []

	for line in lines:
		if _PROC_RE.search(line):
			scope_types = {}
		if _ENDP_RE.search(line):
			scope_types = {}

		var_match = _VAR_TYPE_RE.match(line)
		if var_match:
			var_name, type_name = var_match.groups()
			if type_name in structs:
				scope_types[var_name] = type_name
			output_lines.append(line)
			continue

		line_end = ""
		line_body = line
		if line.endswith("\r\n"):
			line_end = "\r\n"
			line_body = line[:-2]
		elif line.endswith("\n"):
			line_end = "\n"
			line_body = line[:-1]

		code, sep, comment = line_body.partition(";")

		updated_code, replacements = _replace_accesses(code, scope_types, global_types, structs)
		total_replacements += len(replacements)
		if replacements:
			note = _format_replacement_note(replacements)
			if sep:
				if comment.strip():
					comment = f"{comment}  ; {note}"
				else:
					comment = f" {note}"
				output_lines.append(updated_code + sep + comment + line_end)
			else:
				output_lines.append(f"{updated_code} ; {note}{line_end}")
		else:
			output_lines.append(updated_code + sep + comment + line_end)

	if in_place:
		asm_path.write_text("".join(output_lines), encoding="utf-8", errors="surrogateescape")
	elif output_path:
		output_path.write_text("".join(output_lines), encoding="utf-8", errors="surrogateescape")

	return total_replacements


def main() -> None:
	parser = argparse.ArgumentParser(description="Replace MASM struct field accesses with offsets.")
	parser.add_argument("--input", required=True, help="Path to the MASM input file.")
	parser.add_argument("--structs", default='a2server.exe.inc', help="Path to the .inc file with struct definitions.")
	parser.add_argument("--output", help="Path to write the updated MASM output.")
	parser.add_argument("--in-place", action="store_true", help="Update the input file in place.")
	args = parser.parse_args()

	asm_path = Path(args.input)
	inc_path = Path(args.structs)
	output_path = Path(args.output) if args.output else None

	if not asm_path.exists():
		raise SystemExit(f"Input file not found: {asm_path}")
	if not inc_path.exists():
		raise SystemExit(f"Structs file not found: {inc_path}")
	if not args.in_place and output_path is None:
		raise SystemExit("Provide --output or use --in-place.")

	count = replace_offsets(asm_path, inc_path, output_path, args.in_place)
	print(f"Replaced {count} field access(es).")


if __name__ == "__main__":
	main()
