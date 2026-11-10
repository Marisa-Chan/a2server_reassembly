#pragma once

// Logging for the Tangar patch modules, ported from my-srvmgr log.h /
// lib/utils.cpp (Printf) / utils.cpp (log_format, LogString).
//
// Difference from the DLL version: everything is written to the log file
// (Config::LogFile). The DLL additionally routed messages to the server
// console via print_log once the server was started; that routing comes back
// with the redir_log/console patch port.

#include <sstream>
#include <string>

void Printf(const char* format, ...);
void LogString(const std::string& s);

// Logger gathers the message parts in a string stream and the destructor logs
// the accumulated message. Automatically prepends a timestamp and appends a
// newline. Sample usage: Log() << "Hello, " << 42 << "!";
class Log {
public:
    explicit Log() : buffer(std::ostringstream{}) {
    }

    template <typename T>
    Log& operator<<(const T& v) {
        buffer << v;
        return *this;
    }

    ~Log() {
        LogString(buffer.str());
    }

private:
    std::ostringstream buffer;
};
