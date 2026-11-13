#pragma once

// String helpers used by the Tangar patch modules, copied from
// my-srvmgr/lib/utils.{hpp,cpp} (behavior-identical copies).
// One rename: StrToInt -> StrToUInt (the original name is a shlwapi.h macro
// in MFC builds, which the plain DLL build never saw).

#include <string>
#include <vector>

std::string Format(const std::string format, ...);
std::vector<std::string> Explode(const std::string& what, const std::string& separator);
bool IsWhitespace(char what);
std::string TrimLeft(const std::string& what, bool (callback)(char) = IsWhitespace);
std::string TrimRight(const std::string& what, bool (callback)(char) = IsWhitespace);
std::string Trim(const std::string& what, bool (callback)(char) = IsWhitespace);
std::string ToLower(const std::string& what);
std::string ToUpper(const std::string& what);
unsigned long StrToUInt(const std::string& what);
unsigned long HexToInt(const std::string& what);
float StrToFloat(const std::string& what);
bool StrToBool(const std::string& what);
bool CheckInt(const std::string& what);
bool CheckHex(const std::string& what);
bool CheckBool(const std::string& what);
bool CheckFloat(const std::string& what);
bool CheckIP(std::string addr);
void tokenize(std::string const& str, const char delim, std::vector<std::string>& out);
std::string FixSlashes(const std::string& filename);
std::string Basename(const std::string& filename);
