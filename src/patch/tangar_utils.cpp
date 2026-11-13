#include "tangar_utils.h"

#include <cstdarg>
#include <cstdio>
#include <sstream>

// All helpers below are copies from my-srvmgr/lib/utils.cpp.

std::string Format(const std::string format, ...)
{
    char line[2049];
    line[2048] = 0;
    va_list list;
    va_start(list, format);
    vsnprintf(line, 2048, format.c_str(), list);
    va_end(list);

    return std::string(line);
}

std::vector<std::string> Explode(const std::string& what, const std::string& separator)
{
    std::string curstr;
    std::vector<std::string> retval;
    for (size_t i = 0; i < what.length(); i++) {
        if (what.find(separator, i) == i) {
            retval.push_back(curstr);
            curstr.assign("");
            i += separator.length() - 1;
            continue;
        }

        curstr += what[i];
    }
    retval.push_back(curstr);
    return retval;
}

std::string TrimLeft(const std::string& what, bool (callback)(char))
{
    std::string ret = what;
    for (std::string::iterator i = ret.begin(); i != ret.end(); ++i) {
        if (callback((*i))) {
            continue;
        }
        ret.erase(ret.begin(), i);
        return ret;
    }
    return "";
}

std::string TrimRight(const std::string& what, bool (callback)(char))
{
    std::string ret = what;
    for (std::string::reverse_iterator i = ret.rbegin(); i != ret.rend(); ++i) {
        if (callback((*i))) {
            continue;
        }
        ret.erase(i.base(), ret.end());
        return ret;
    }
    return "";
}

std::string Trim(const std::string& what, bool (callback)(char))
{
    return TrimRight(TrimLeft(what, callback));
}

bool IsWhitespace(char what)
{
    switch ((unsigned char)what) {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case 0xFF:
            return true;
        default:
            return false;
    }
}

std::string ToLower(const std::string& what)
{
    std::string ret = what;
    for (std::string::iterator i = ret.begin(); i != ret.end(); ++i) {
        (*i) = tolower((*i));
    }
    return ret;
}

std::string ToUpper(const std::string& what)
{
    std::string ret = what;
    for (std::string::iterator i = ret.begin(); i != ret.end(); ++i) {
        (*i) = toupper((*i));
    }
    return ret;
}

unsigned long StrToUInt(const std::string& what)
{
    unsigned long retval;
    sscanf(what.c_str(), "%u", &retval);
    return retval;
}

unsigned long HexToInt(const std::string& what)
{
    unsigned int retval;
    sscanf(what.c_str(), "%X", &retval);
    return retval;
}

bool CheckHex(const std::string& what)
{
    for (size_t i = 0; i < what.length(); i++) {
        if ((what[i] < 0x30 || 0x39 < what[i]) && !(what[i] >= 'A' && what[i] <= 'F') && !(what[i] >= 'a' && what[i] <= 'f')) {
            return false;
        }
    }
    return true;
}

float StrToFloat(const std::string& what)
{
    float retval;
    sscanf(what.c_str(), "%f", &retval);
    return retval;
}

// Take each char from (hopefully) numeric string and check that each character
// is ASCII 48 ('0') to 57 ('9').
bool CheckInt(const std::string& what)
{
    for (size_t i = 0; i < what.length(); i++) {
        if (what[i] < 0x30 || 0x39 < what[i]) {
            return false;
        }
    }
    return true;
}

bool CheckBool(const std::string& what)
{
    std::string wh2 = ToLower(Trim(what));
    if (wh2 == "true" || wh2 == "false" || wh2 == "yes" || wh2 == "no" || wh2 == "y" || wh2 == "n" || wh2 == "0" || wh2 == "1") {
        return true;
    }
    return false;
}

bool StrToBool(const std::string& what)
{
    std::string cr = Trim(ToLower(what));
    if (cr == "yes" || cr == "true" || cr == "1" || cr == "y") {
        return true;
    }
    return false;
}

bool CheckIP(std::string addr)
{
    std::vector<std::string> addr2 = Explode(addr, ".");
    if (!((addr2.size() == 4) && CheckInt(addr2[0]) && CheckInt(addr2[1]) && CheckInt(addr2[2]) && CheckInt(addr2[3]))) {
        return false;
    }
    uint32_t oct[4];
    oct[0] = StrToUInt(addr2[0]);
    oct[1] = StrToUInt(addr2[1]);
    oct[2] = StrToUInt(addr2[2]);
    oct[3] = StrToUInt(addr2[3]);
    if (oct[0] > 255 || oct[1] > 255 || oct[2] > 255 || oct[3] > 255) {
        return false;
    }
    return true;
}

bool CheckFloat(const std::string& what)
{
    for (size_t i = 0; i < what.length(); i++) {
        if ((what[i] < 0x30 || 0x39 < what[i]) && what[i] != '.') {
            return false;
        }
    }
    return true;
}

void tokenize(std::string const& str, const char delim, std::vector<std::string>& out)
{
    // Construct a stream from the string and split it by the delimiter.
    std::stringstream ss(str);

    std::string s;
    while (std::getline(ss, s, delim)) {
        out.push_back(s);
    }
}

std::string FixSlashes(const std::string& filename)
{
    std::string ret = filename;
    for (char& ch : ret) {
        if (ch == '\\') {
            ch = '/';
        }
    }
    return ret;
}

std::string Basename(const std::string& filename)
{
    std::string ret = FixSlashes(filename);
    size_t where = ret.find_last_of('/');
    if (where == std::string::npos) {
        return ret;
    }
    ret.erase(0, where + 1);
    return ret;
}
