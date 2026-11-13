#pragma once

namespace tangar_crash {

// Install the top-level unhandled SEH exception filter (ported from
// my-srvmgr/crash_filter.cpp + DllMain emergency character save).
void SetExceptionFilter();

} // namespace tangar_crash
