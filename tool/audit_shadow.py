import re
import subprocess
import sys
import collections

BACK = chr(92)
sites = set()
for ln in open(sys.argv[1], encoding='utf-8', errors='replace'):
    ln = ln.replace(BACK, '/')
    m = re.search(r'src/([A-Za-z_0-9]+\.(?:cpp|h))\((\d+),\d+\): error C4458: '
                  r"declaration of '([A-Za-z_0-9]+)' hides", ln)
    if m:
        sites.add((m.group(1), int(m.group(2)), m.group(3)))
print('unique shadow sites:', len(sites))


def original(fname):
    return subprocess.run(['git', 'show', 'HEAD:src/' + fname],
                          capture_output=True, text=True,
                          encoding='utf-8', errors='replace').stdout


bad = 0
for fname, lineno, name in sorted(sites):
    lines = original(fname).split('\n')
    fstart = 0
    for i in range(lineno - 1, -1, -1):
        if re.match(r'^[A-Za-z_].*::[A-Za-z_~]+\(', lines[i]):
            fstart = i
            break
    depth = 0
    fend = len(lines) - 1
    started = False
    for i in range(fstart, len(lines)):
        depth += lines[i].count('{') - lines[i].count('}')
        if '{' in lines[i]:
            started = True
        if started and depth <= 0:
            fend = i
            break
    # added lines of this function in the working-tree diff
    diff = subprocess.run(
        ['git', 'diff', '-U0', 'HEAD', '--', 'src/' + fname],
        capture_output=True, text=True, encoding='utf-8',
        errors='replace').stdout
    cur = 0
    added = []
    for dl in diff.split('\n'):
        hm = re.match(r'@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@', dl)
        if hm:
            cur = int(hm.group(1))
            continue
        if dl.startswith('+') and not dl.startswith('+++'):
            added.append((cur, dl[1:]))
            cur += 1
        elif dl.startswith('-') or dl.startswith(' '):
            pass
    # map added line numbers back: does any added line fall inside the
    # original function span and contain this->name?
    for aline, atext in added:
        if fstart + 1 <= aline <= fend + 1 and \
                re.search(r'this->%s\b' % re.escape(name), atext):
            # was the same this->name present in the original line?
            print('CHECK %s orig-func %d-%d shadow %s at %d; added line %d: %s'
                  % (fname, fstart + 1, fend + 1, name, lineno, aline,
                     atext.strip()))
            bad += 1
print('added-line hits needing eyeball:', bad)
