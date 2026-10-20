#!/bin/bash
# Gate a rename commit on a green build.
# usage: tool/commit_rename.sh "<commit message>" "<rename.txt line>"
# Stages all tracked modified files (git add -u); leaves untracked files alone.
set -u
cd "$(dirname "$0")/.."
msg="$1"; line="$2"
powershell -NoProfile -ExecutionPolicy Bypass -File tool/build.ps1 > /tmp/build.log 2>&1
s=$?
if [ $s -eq 0 ] && ! grep -qE 'error C[0-9]+|error LNK[0-9]+|fatal error' /tmp/build.log; then
    printf '%s\n' "$line" >> migration_state/rename.txt
    git add -u
    git commit -q -m "$msg"
    echo "OK: $msg"
else
    echo "BUILD_FAILED: $msg"
    grep -E 'error' /tmp/build.log | head -5
    exit 1
fi
