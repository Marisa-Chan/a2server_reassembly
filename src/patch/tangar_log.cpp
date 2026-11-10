#include "tangar_log.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <windows.h>

#include "config_new.h"

// Writes one timestamped line to Config::LogFile (appended). Ported from
// my-srvmgr utils.cpp log_format; the KOI8/CP1251 output-encoding conversion
// is dropped (it only affected Cyrillic log text).
static void log_format(const char* format, ...)
{
    char line[4096], l2[4096];
    va_list va;
    va_start(va, format);
    _vsnprintf(line, sizeof(line) - 1, format, va);
    va_end(va);
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    _snprintf(l2, sizeof(l2) - 1, "[%d.%.2d.%.4d %d:%.2d:%.2d.%.3d] %s", tm.wDay, tm.wMonth, tm.wYear,
              tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, line);
    FILE* fp = fopen(Config::LogFile.c_str(), "a");
    if (fp != nullptr) {
        fprintf(fp, "%s", l2);
        fclose(fp);
    }
}

void LogString(const std::string& s)
{
    log_format("%s\n", s.c_str());
}

void Printf(const char* format, ...)
{
    char line[4096];
    va_list list;
    va_start(list, format);
    vsnprintf(line, sizeof(line) - 1, format, list);
    va_end(list);

    log_format("%s\n", line);
}
