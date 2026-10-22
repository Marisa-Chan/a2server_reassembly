#!/usr/bin/env python
"""Standalone Ghidra daemon: opens the A2 project with pyghidra and serves BOTH
programs (server + client) over HTTP for tool/gh.py (ops in tool/ghidra_ops.py).

    python tool/ghidra_daemon.py                     # server default; client opens on first 'gh.py --client'
    python tool/ghidra_daemon.py --no-autosave
    python tool/ghidra_daemon.py --program client    # client is the default (legacy single-program style)
    python tool/ghidra_daemon.py --preload client    # open the client eagerly instead of on first use
    python tool/ghidra_daemon.py --project C:\\path\\to\\dir --name A2

The Ghidra GUI must NOT have the same project open (it holds the project lock).
Stop with Ctrl+C or 'python tool/gh.py shutdown'; programs are saved on exit
(local checkout only - check in from the Ghidra GUI to publish to the shared server).
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
PROGRAMS = {"server": "/a2serv6.exe_new", "client": "/allods2.exe"}


def main():
    ghidra_ops = _load_ops()
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--project", default=PROJECT_DIR)
    ap.add_argument("--name", default=PROJECT_NAME)
    ap.add_argument("--program", help="which program is the DEFAULT for requests that omit one "
                    "(server/client/a2serv6.exe_new/allods2.exe); both stay reachable either way")
    ap.add_argument("--preload", action="append", default=[], metavar="NAME",
                    help="open this program eagerly at startup instead of on first use (repeatable); "
                    "the default program is always opened eagerly")
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
    registry = ghidra_ops.ProgramRegistry(
        PROGRAMS, "server",
        opener=lambda path: pyghidra.consume_program(project, path),
        autosave=not a.no_autosave)
    if a.program:
        registry.default = registry.resolve(a.program)  # legacy flag: used to pick THE only program
    registry.get(registry.default)
    for name in a.preload:
        registry.get(name)
    print(f"[ghidra_daemon] ready in {time.time() - t0:.1f}s; stop with Ctrl+C or 'python tool/gh.py shutdown'")
    try:
        ghidra_ops.serve(registry, port=a.port)
    finally:
        try:
            saved = registry.save_all("ghidra_daemon exit")
            if saved:
                print(f"[ghidra_daemon] saved: {', '.join(saved)}")
        finally:
            registry.release_all()
            project.close()
            print("[ghidra_daemon] closed")


if __name__ == "__main__":
    main()
