#pragma once

// Logging for the Tangar patch modules, ported from my-srvmgr log.h /
// lib/utils.cpp (Printf) / utils.cpp (log_format, LogString).
//
// Once Config::ServerStarted is true, Printf and LogString route lines through
// PostConsoleLine (print_log @ 43A857), which both appends to the server GUI
// console and writes the timestamped line to Config::LogFile via redir_log.

#include <sstream>
#include <string>

void log_format(const char* format, ...);
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
