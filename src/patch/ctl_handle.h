#pragma once

namespace tangar_ctl {

// Start/stop the background control-directory watcher thread
// (ported from my-srvmgr/ctl_handle.cpp and srvmgr.cpp DllMain).
void StartControlThread();
void StopControlThread();

// Drain queued say.* and cheat.* control actions on the main server thread.
void PollControlActions();

// True while PollControlActions is dispatching a cheat.* command on behalf of a
// named player (matching RunCommand(..., rights = 0xFFFFFFFF, console = false)).
bool IsFullRightsOverride();

} // namespace tangar_ctl
