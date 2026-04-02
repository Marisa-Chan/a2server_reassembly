"""
Find all sections of a function in Main.asm: proc body, FUNCTION CHUNKs, and SEH .rdata entries.

Usage:
    python asm_find_func.py <hex_addr>              # e.g. python asm_find_func.py 560DC2
    python asm_find_func.py <hex_addr> [asm_file]   # custom path to Main.asm
"""
import sys
import os
import re


def main():
    if len(sys.argv) < 2:
        print("Usage: python asm_find_func.py <hex_address> [asm_file]")
        sys.exit(1)

    address = sys.argv[1]
    if len(sys.argv) >= 3:
        asm_file = sys.argv[2]
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        asm_file = os.path.join(script_dir, "..", "src", "Main.asm")

    asm_file = os.path.abspath(asm_file)
    if not os.path.isfile(asm_file):
        print(f"ERROR: File not found: {asm_file}")
        sys.exit(1)

    func_name = f"sub_{address}"

    with open(asm_file, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    total = len(lines)
    print(f"Searching {total} lines for {func_name} ...")

    # --- 1. Find proc/endp ---
    proc_line = -1
    for i, line in enumerate(lines):
        if re.search(rf"^\s*{re.escape(func_name)}\s+proc\b", line):
            proc_line = i + 1  # 1-based
            break

    if proc_line == -1:
        print(f"ERROR: '{func_name} proc' not found.")
        sys.exit(1)

    endp_line = -1
    for i in range(proc_line, total):  # proc_line is already 1-based, so index = proc_line
        if re.search(r"\bendp\b", lines[i]):
            endp_line = i + 1  # 1-based
            break

    if endp_line == -1:
        print("ERROR: matching 'endp' not found.")
        sys.exit(1)

    # Extend range to include separator comment above proc
    start_line = proc_line
    for j in range(proc_line - 2, max(-1, proc_line - 6), -1):
        if re.search(r"^;\s*-{10,}", lines[j]):
            start_line = j + 1  # 1-based
            break

    print()
    print("=== PROC BODY ===")
    print(f"  Lines: {start_line} - {endp_line}  ({endp_line - start_line + 1} lines)")
    print(f"  First: {lines[start_line - 1].rstrip()}")
    print(f"  Last:  {lines[endp_line - 1].rstrip()}")

    # Check for jump table immediately after endp
    jump_end = endp_line
    for i in range(endp_line, min(total, endp_line + 500)):
        line = lines[i].strip()
        if re.search(r"^;\s*-{10,}", line) or re.search(r"\bproc\b", line) or re.search(r"^;.*FUNCTION CHUNK", line):
            break
        if re.search(r"^dd\s+offset", line) or line == "" or re.search(r"^;\s*jumptable", line):
            jump_end = i + 1  # 1-based

    if jump_end > endp_line:
        print()
        print("=== JUMP TABLE ===")
        print(f"  Lines: {endp_line + 1} - {jump_end}  ({jump_end - endp_line} lines)")
        endp_line = jump_end
        print(f"  Combined proc+jump range: {start_line} - {endp_line}")

    # --- 2. Find FUNCTION CHUNK FOR this function ---
    chunks = []
    escaped_func = re.escape(func_name)
    for i in range(total):
        if re.search(rf"FUNCTION CHUNK FOR\s+{escaped_func}\b", lines[i]):
            chunk_start = i + 1  # 1-based
            chunk_end = chunk_start
            for j in range(i + 1, min(total, i + 200)):
                if re.search(rf"END OF FUNCTION CHUNK FOR\s+{escaped_func}\b", lines[j]):
                    chunk_end = j + 1  # 1-based
                    break

            # Include separator above
            cs_line = chunk_start
            for j in range(i - 1, max(-1, i - 5), -1):
                if re.search(r"^;\s*-{10,}", lines[j]):
                    cs_line = j + 1
                    break

            # Include separator below
            ce_line = chunk_end
            if ce_line < total and re.search(r"^;\s*-{10,}", lines[ce_line]):
                ce_line += 1

            chunks.append((cs_line, ce_line))

    if chunks:
        print()
        print("=== FUNCTION CHUNKS ===")
        for cs, ce in chunks:
            print(f"  Lines: {cs} - {ce}  ({ce - cs + 1} lines)")

    # --- 3. Find SEH .rdata structures ---
    stru_names = []
    body_start = proc_line - 1  # 0-based
    body_end = min(endp_line, total) - 1

    for i in range(body_start, body_end + 1):
        m = re.search(r"\b(stru_[0-9A-Fa-f]+)\b", lines[i])
        if m and m.group(1) not in stru_names:
            stru_names.append(m.group(1))

    for cs, ce in chunks:
        for i in range(cs - 1, ce):
            m = re.search(r"\b(stru_[0-9A-Fa-f]+)\b", lines[i])
            if m and m.group(1) not in stru_names:
                stru_names.append(m.group(1))

    if stru_names:
        print()
        print("=== SEH .rdata STRUCTURES ===")
        print(f"  Referenced: {', '.join(stru_names)}")

        for sn in stru_names:
            escaped_sn = re.escape(sn)
            for i in range(total):
                if re.search(rf"^\s*{escaped_sn}\b.*\bdd\b", lines[i]):
                    seh_start = i + 1
                    seh_end = seh_start
                    for j in range(i + 1, min(total, i + 50)):
                        sl = lines[j].strip()
                        if re.search(r"^\s*dd\s+", sl) or re.search(r"^\s*db\s+", sl) or sl == "":
                            seh_end = j + 1
                        else:
                            break
                    print(f"  {sn} : Lines {seh_start} - {seh_end}  ({seh_end - seh_start + 1} lines)")
                    break

    # --- Summary ---
    print()
    print("=== DELETION SUMMARY ===")
    print(f"  Proc body:  {start_line} - {endp_line}")
    for cs, ce in chunks:
        print(f"  Chunk:      {cs} - {ce}")

    ranges = [f"{start_line}-{endp_line}"]
    for cs, ce in chunks:
        ranges.append(f"{cs}-{ce}")

    print()
    print("To delete, run:")
    print(f"  python asm_delete_lines.py {' '.join(ranges)}")
    print()
    print("NOTE: SEH .rdata entries listed above may need manual review")
    print("      (they can be shared with other functions).")


if __name__ == "__main__":
    main()
