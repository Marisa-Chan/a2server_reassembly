#include "tangar_log.h"

#include "../game_app.h"
#include "config_new.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>

// Writes one timestamped line to Config::LogFile (appended). Ported from
// my-srvmgr utils.cpp log_format; the KOI8/CP1251 output-encoding conversion
// is dropped (it only affected Cyrillic log text).
void log_format(const char* format, ...)
{
    char line[4096], l2[4096];
    va_list va;
    va_start(va, format);
    _vsnprintf(line, sizeof(line) - 1, format, va);
    line[sizeof(line) - 1] = '\0';
    va_end(va);
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    _snprintf(l2, sizeof(l2) - 1, "[%d.%.2d.%.4d %d:%.2d:%.2d.%.3d] %s", tm.wDay, tm.wMonth, tm.wYear,
              tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, line);
    l2[sizeof(l2) - 1] = '\0';
    FILE* fp = fopen(Config::LogFile.c_str(), "a");
    if (fp != nullptr) {
        fprintf(fp, "%s", l2);
        fclose(fp);
    }
}

void LogString(const std::string& s)
{
    if (Config::ServerStarted) {
        PostConsoleLine(s.c_str());
    } else {
        log_format("%s\n", s.c_str());
    }
}

void Printf(const char* format, ...)
{
    char line[4096];
    va_list list;
    va_start(list, format);
    vsnprintf(line, sizeof(line) - 1, format, list);
    line[sizeof(line) - 1] = '\0';
    va_end(list);

    if (Config::ServerStarted) {
        PostConsoleLine(line);
    } else {
        log_format("%s\n", line);
    }
}
