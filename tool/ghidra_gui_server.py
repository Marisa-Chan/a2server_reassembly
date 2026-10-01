# Serve the currently open program to tool/gh.py from inside the Ghidra GUI.
# Requires Ghidra started via support\pyghidraRun.bat (so Python scripts run).
# Add this repo's tool/ folder to Script Manager > "Manage Script Directories",
# then run this script; stop it with the Script Manager's cancel button.
# Changes are NOT auto-saved: save in Ghidra with Ctrl+S as usual.
#@category A2
#@runtime PyGhidra
import importlib.util
import os

_spec = importlib.util.spec_from_file_location(
    "ghidra_ops", os.path.join(os.path.dirname(os.path.abspath(__file__)), "ghidra_ops.py"))
ghidra_ops = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(ghidra_ops)

ctx = ghidra_ops.OpContext(currentProgram, autosave=False)
ghidra_ops.serve(ctx, should_stop=lambda: monitor.isCancelled(), log=println)
