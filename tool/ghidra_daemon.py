#!/usr/bin/env python
"""Standalone Ghidra daemon: opens the A2 project with pyghidra and serves
tool/ghidra_ops.py over HTTP for tool/gh.py.

    python tool/ghidra_daemon.py                 # uses defaults below
    python tool/ghidra_daemon.py --no-autosave
    python tool/ghidra_daemon.py --project C:\\path\\to\\dir --name A2 --program /a2serv6.exe_new

The Ghidra GUI must NOT have the same project open (it holds the project lock).
Stop with Ctrl+C; the program is saved on exit (local checkout only - check in
from the Ghidra GUI when you want to publish to the shared server).
"""
import argparse
import importlib.util
import os
import time

HERE = os.path.dirname(os.path.abspath(__file__))


def _load_ops():
    # not via sys.path: tool/ contains other modules that could shadow Java packages
    spec = importlib.util.spec_from_file_location("ghidra_ops", os.path.join(HERE, "ghidra_ops.py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _prefs_path():
    from ghidra.framework import Application
    return os.path.join(str(Application.getUserSettingsDirectory().getAbsolutePath()), "preferences")


def _read_prefs():
    try:
        with open(_prefs_path(), encoding="utf-8") as fh:
            return fh.read()
    except OSError:
        return None


def _restore_prefs(old):
    if old is None:
        return
    from ghidra.framework.preferences import Preferences
    import re
    for key in ("LastOpenedProject", "ProjectDirectory", "RecentProjects"):
        m = re.search(rf"^{key}=(.*)$", old, re.M)
        if m:
            Preferences.setProperty(key, m.group(1).replace("\\\\", "\\").replace("\\:", ":"))
    Preferences.store()

# Ghidra install and the project checkout are siblings of this repo (../ghidra_12.1.2_PUBLIC, ../shared-ghidra)
A2_DIR = os.path.dirname(os.path.dirname(HERE))
GHIDRA_INSTALL_DIR = os.path.join(A2_DIR, "ghidra_12.1.2_PUBLIC")
PROJECT_DIR = os.path.join(A2_DIR, "shared-ghidra")
PROJECT_NAME = "A2"
PROGRAM_PATH = "/a2serv6.exe_new"


def main():
    ghidra_ops = _load_ops()
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--project", default=PROJECT_DIR)
    ap.add_argument("--name", default=PROJECT_NAME)
    ap.add_argument("--program", default=PROGRAM_PATH)
    ap.add_argument("--port", type=int, default=ghidra_ops.DEFAULT_PORT)
    ap.add_argument("--no-autosave", action="store_true", help="only save on 'save' op and on exit")
    ap.add_argument("--ghidra", default=os.environ.get("GHIDRA_INSTALL_DIR", GHIDRA_INSTALL_DIR))
    a = ap.parse_args()

    lock = os.path.join(a.project, a.name + ".lock")
    if os.path.exists(lock):
        print(f"[ghidra_daemon] WARNING: {lock} exists - is the Ghidra GUI still open on this project?")

    os.environ["GHIDRA_INSTALL_DIR"] = a.ghidra
    import pyghidra
    t0 = time.time()
    pyghidra.start(verbose=False)
    prefs = _read_prefs()
    project = pyghidra.open_project(a.project, a.name)
    _restore_prefs(prefs)  # opening a project rewrites LastOpenedProject; don't redirect the GUI
    program, consumer = pyghidra.consume_program(project, a.program)
    print(f"[ghidra_daemon] opened {program.getName()} ({program.getFunctionManager().getFunctionCount()} functions) in {time.time() - t0:.1f}s")
    ctx = ghidra_ops.OpContext(program, autosave=not a.no_autosave)
    try:
        ghidra_ops.serve(ctx, port=a.port)
    finally:
        try:
            if ctx.save("ghidra_daemon exit"):
                print("[ghidra_daemon] saved")
        finally:
            program.release(consumer)
            project.close()
            print("[ghidra_daemon] closed")


if __name__ == "__main__":
    main()
