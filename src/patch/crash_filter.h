#pragma once

namespace tangar_crash {

// Install the top-level unhandled SEH exception filter (ported from
// srvmgr/crash_filter.cpp + DllMain emergency character save).
void SetExceptionFilter();

// Write a minidump + stack trace for the current thread without terminating
// the process - for non-crash failures (e.g. suppressed MFC error boxes).
void WriteDiagnostic(const char* reason);

} // namespace tangar_crash
