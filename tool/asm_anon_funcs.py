"""
Find all anonymous (unnamed) functions in Main.asm.

Anonymous functions are code blocks that sit between named proc/endp pairs and
contain executable instructions but have no `proc near/far` label of their own.

Usage:
    .\asm_anon_funcs.py                         # scan default Main.asm
    .\asm_anon_funcs.py [asm_file]              # custom path to Main.asm
    .\asm_anon_funcs.py --delete                # delete all anonymous functions
"""
import sys
import os
import re
import argparse


PROC_RE      = re.compile(r'\bproc\s+(?:near|far)\b')
ENDP_RE      = re.compile(r'^\S+\s+endp\b')
DIRECTIVE_RE = re.compile(r'^(?:align|assume|db|dw|dd|dq|byte|word|dword|qword|offset|include|option)\b', re.IGNORECASE)
VARDEF_RE    = re.compile(r'^\w+\s*=\s*')
LABEL_ONLY_RE = re.compile(r'^\w[^;]*:\s*$')


def is_instruction(line: str) -> bool:
    """Return True if the line contains executable code (not a comment, blank, directive, or label-only line)."""
    if not line[0:1].isspace():
        return False
    stripped = line.lstrip()
    if not stripped or stripped.startswith(';'):
        return False
    if stripped.startswith('.'):   # dot-directives: .686p, .mmx, .model, .code, .data, etc.
        return False
    if DIRECTIVE_RE.match(stripped):
        return False
    if VARDEF_RE.match(stripped):
        return False
    if LABEL_ONLY_RE.match(stripped):
        return False
    return True


def find_anonymous_functions(asm_file: str):
    results = []
    in_proc = False
    gap_lines = []   # (lineno, text) collected since last endp

    def finalize_gap():
        instr_lines = [(n, t) for n, t in gap_lines if is_instruction(t)]
        if instr_lines:
            first_n, first_t = instr_lines[0]
            last_n, _ = instr_lines[-1]
            results.append((first_n, last_n, first_t.strip()))

    with open(asm_file, 'r', encoding='utf-8', errors='replace') as f:
        for lineno, raw in enumerate(f, 1):
            line = raw.rstrip('\n')

            if 'START OF FUNCTION CHUNK' in line:
                break

            if PROC_RE.search(line):
                if not in_proc and gap_lines:
                    finalize_gap()
                gap_lines = []
                in_proc = True

            elif ENDP_RE.match(line):
                in_proc = False
                gap_lines = []

            elif not in_proc:
                gap_lines.append((lineno, line))

    # Catch any trailing anonymous block after the last endp
    if not in_proc and gap_lines:
        finalize_gap()

    return results


def delete_ranges(asm_file: str, results: list):
    with open(asm_file, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    original_count = len(lines)
    total_deleted = 0

    # Process bottom-up so earlier line numbers stay valid
    for start, end, preview in reversed(results):
        count = end - start + 1
        si = start - 1  # 0-based inclusive
        print(f'  Delete lines {start}-{end} ({count} lines): {preview}')
        del lines[si:end]  # end is 1-based, so si:end deletes si..end-1 i.e. si..ei inclusive
        total_deleted += count

    print()
    with open(asm_file, 'w', encoding='utf-8', newline='') as f:
        f.writelines(lines)
    print(f'Deleted {len(results)} anonymous function(s), {total_deleted} lines. '
            f'File now has {len(lines)} lines (was {original_count}).')


def main():
    parser = argparse.ArgumentParser(description='Find (and optionally delete) anonymous functions in Main.asm')
    parser.add_argument('asm_file', nargs='?', default=None, help='Path to Main.asm (default: ../src/Main.asm)')
    parser.add_argument('--delete', action='store_true', help='Delete all anonymous functions from the file')
    args = parser.parse_args()

    if args.asm_file:
        asm_file = args.asm_file
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        asm_file = os.path.join(script_dir, '..', 'src', 'Main.asm')

    asm_file = os.path.abspath(asm_file)
    if not os.path.isfile(asm_file):
        print(f'ERROR: File not found: {asm_file}')
        sys.exit(1)

    print(f'Scanning: {asm_file}')
    results = find_anonymous_functions(asm_file)

    if not results:
        print('No anonymous functions found.')
        return

    if args.delete:
        delete_ranges(asm_file, results)
    else:
        print(f'Found {len(results)} anonymous function(s):\n')
        total_lines = 0
        for start, end, preview in results:
            print(f'Lines {start}-{end}: {preview}')
            total_lines += end - start + 1
        print(f'\nTotal: {len(results)} anonymous function(s), {total_lines} lines')


if __name__ == '__main__':
    main()
