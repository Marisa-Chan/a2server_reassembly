/*
 * HappyHTTP - a simple HTTP library
 * Version 0.1
 *
 * Copyright (c) 2006 Ben Campbell
 *
 * Vendored from my-a2mgr/lib/happyhttp.cpp (altered for MSVC _WIN32 / VS2022).
 */

#include "happyhttp.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cctype>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

namespace happyhttp {

#ifdef _WIN32
const char* GetWinsockErrorString(int err);
#endif

void BailOnSocketError(const char* context) {
#ifdef _WIN32
    int e = WSAGetLastError();
    const char* msg = GetWinsockErrorString(e);
#else
    const char* msg = strerror(errno);
#endif
    throw Wobbly("%s: %s", context, msg);
}

#ifdef _WIN32
const char* GetWinsockErrorString(int err) {
    switch (err) {
    case 0: return "No error";
    case WSAEINTR: return "Interrupted system call";
    case WSAEBADF: return "Bad file number";
    case WSAEACCES: return "Permission denied";
    case WSAEFAULT: return "Bad address";
    case WSAEINVAL: return "Invalid argument";
    case WSAEMFILE: return "Too many open sockets";
    case WSAEWOULDBLOCK: return "Operation would block";
    case WSAEINPROGRESS: return "Operation now in progress";
    case WSAEALREADY: return "Operation already in progress";
    case WSAENOTSOCK: return "Socket operation on non-socket";
    case WSAEDESTADDRREQ: return "Destination address required";
    case WSAEMSGSIZE: return "Message too long";
    case WSAEPROTOTYPE: return "Protocol wrong type for socket";
    case WSAENOPROTOOPT: return "Bad protocol option";
    case WSAEPROTONOSUPPORT: return "Protocol not supported";
    case WSAESOCKTNOSUPPORT: return "Socket type not supported";
    case WSAEOPNOTSUPP: return "Operation not supported on socket";
    case WSAEPFNOSUPPORT: return "Protocol family not supported";
    case WSAEAFNOSUPPORT: return "Address family not supported";
    case WSAEADDRINUSE: return "Address already in use";
    case WSAEADDRNOTAVAIL: return "Can't assign requested address";
    case WSAENETDOWN: return "Network is down";
    case WSAENETUNREACH: return "Network is unreachable";
    case WSAENETRESET: return "Net connection reset";
    case WSAECONNABORTED: return "Software caused connection abort";
    case WSAECONNRESET: return "Connection reset by peer";
    case WSAENOBUFS: return "No buffer space available";
    case WSAEISCONN: return "Socket is already connected";
    case WSAENOTCONN: return "Socket is not connected";
    case WSAESHUTDOWN: return "Can't send after socket shutdown";
    case WSAETOOMANYREFS: return "Too many references, can't splice";
    case WSAETIMEDOUT: return "Connection timed out";
    case WSAECONNREFUSED: return "Connection refused";
    case WSAELOOP: return "Too many levels of symbolic links";
    case WSAENAMETOOLONG: return "File name too long";
    case WSAEHOSTDOWN: return "Host is down";
    case WSAEHOSTUNREACH: return "No route to host";
    case WSAENOTEMPTY: return "Directory not empty";
    case WSAEPROCLIM: return "Too many processes";
    case WSAEUSERS: return "Too many users";
    case WSAEDQUOT: return "Disc quota exceeded";
    case WSAESTALE: return "Stale NFS file handle";
    case WSAEREMOTE: return "Too many levels of remote in path";
    case WSASYSNOTREADY: return "Network system is unavailable";
    case WSAVERNOTSUPPORTED: return "Winsock version out of range";
    case WSANOTINITIALISED: return "WSAStartup not yet called";
    case WSAEDISCON: return "Graceful shutdown in progress";
    case WSAHOST_NOT_FOUND: return "Host not found";
    case WSANO_DATA: return "No host data of that type was found";
    }
    return "unknown";
}
#endif

static bool datawaiting(int sock) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(static_cast<SOCKET>(sock), &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    int r = select(sock + 1, &fds, NULL, NULL, &tv);
    if (r < 0) {
        BailOnSocketError("select");
    }
    return FD_ISSET(static_cast<SOCKET>(sock), &fds) != 0;
}

struct in_addr* atoaddr(const char* address) {
    static struct in_addr saddr;

    if (inet_pton(AF_INET, address, &saddr) == 1) {
        return &saddr;
    }

    struct addrinfo hints = {};
    struct addrinfo* result = nullptr;
    hints.ai_family = AF_INET;

    int err = getaddrinfo(address, nullptr, &hints, &result);
    if (err != 0 || !result) {
        return nullptr;
    }

    struct sockaddr_in* ipv4 = (struct sockaddr_in*)result->ai_addr;
    saddr = ipv4->sin_addr;

    freeaddrinfo(result);
    return &saddr;
}

Wobbly::Wobbly(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf_s(m_Message, MAXLEN, _TRUNCATE, fmt, ap);
    va_end(ap);
    if (n == MAXLEN) {
        m_Message[MAXLEN - 1] = '\0';
    }
}

Connection::Connection(const char* host, int port) :
    m_ResponseBeginCB(0),
    m_ResponseDataCB(0),
    m_ResponseCompleteCB(0),
    m_UserData(0),
    m_State(IDLE),
    m_Host(host),
    m_Port(port),
    m_Sock(-1)
{
}

void Connection::setcallbacks(
    ResponseBegin_CB begincb,
    ResponseData_CB datacb,
    ResponseComplete_CB completecb,
    void* userdata)
{
    m_ResponseBeginCB = begincb;
    m_ResponseDataCB = datacb;
    m_ResponseCompleteCB = completecb;
    m_UserData = userdata;
}

void Connection::connect() {
    in_addr* addr = atoaddr(m_Host.c_str());
    if (!addr) {
        throw Wobbly("Invalid network address");
    }

    sockaddr_in address;
    memset((char*)&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<u_short>(m_Port));
    address.sin_addr.s_addr = addr->s_addr;

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        BailOnSocketError("socket()");
    }
    m_Sock = static_cast<int>(s);

    // Use a 4-second non-blocking connect timeout so an unreachable update/API
    // server fails fast without freezing the login UI for 21 seconds.
    u_long nonblock = 1;
    ioctlsocket(s, FIONBIO, &nonblock);
    int rc = ::connect(s, (sockaddr const*)&address, sizeof(address));
    if (rc == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
            closesocket(s);
            m_Sock = -1;
            WSASetLastError(err);
            BailOnSocketError("connect()");
        }
        fd_set wfds;
        fd_set efds;
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        FD_SET(s, &wfds);
        FD_SET(s, &efds);
        timeval tv;
        tv.tv_sec = 4;
        tv.tv_usec = 0;
        int sel = select(0, nullptr, &wfds, &efds, &tv);
        if (sel <= 0 || FD_ISSET(s, &efds) || !FD_ISSET(s, &wfds)) {
            closesocket(s);
            m_Sock = -1;
            WSASetLastError(WSAETIMEDOUT);
            BailOnSocketError("connect()");
        }
    }
    nonblock = 0;
    ioctlsocket(s, FIONBIO, &nonblock);
}

void Connection::close() {
#ifdef _WIN32
    if (m_Sock >= 0) {
        ::closesocket(static_cast<SOCKET>(m_Sock));
    }
#else
    if (m_Sock >= 0) {
        ::close(m_Sock);
    }
#endif
    m_Sock = -1;

    while (!m_Outstanding.empty()) {
        delete m_Outstanding.front();
        m_Outstanding.pop_front();
    }
}

Connection::~Connection() {
    close();
}

void Connection::request(const char* method,
    const char* url,
    const char* headers[],
    const unsigned char* body,
    int bodysize)
{
    bool gotcontentlength = false;

    if (headers) {
        const char** h = headers;
        while (*h) {
            const char* name = *h++;
            const char* value = *h++;
            assert(value != 0);
            if (0 == strcasecmp(name, "content-length")) {
                gotcontentlength = true;
            }
        }
    }

    putrequest(method, url);

    if (body && !gotcontentlength) {
        putheader("Content-Length", bodysize);
    }

    if (headers) {
        const char** h = headers;
        while (*h) {
            const char* name = *h++;
            const char* value = *h++;
            putheader(name, value);
        }
    }
    endheaders();

    if (body) {
        send(body, bodysize);
    }
}

void Connection::putrequest(const char* method, const char* url) {
    if (m_State != IDLE) {
        throw Wobbly("Request already issued");
    }

    m_State = REQ_STARTED;

    char req[512];
    sprintf_s(req, sizeof(req), "%s %s HTTP/1.1", method, url);

    m_Buffer.push_back(req);
    putheader("Host", m_Host.c_str());
    putheader("Accept-Encoding", "identity");

    Response* r = new Response(method, *this);
    m_Outstanding.push_back(r);
}

void Connection::putheader(const char* header, const char* value) {
    if (m_State != REQ_STARTED) {
        throw Wobbly("putheader() failed");
    }
    m_Buffer.push_back(string(header) + ": " + string(value));
}

void Connection::putheader(const char* header, int numericvalue) {
    char buf[32];
    sprintf_s(buf, sizeof(buf), "%d", numericvalue);
    putheader(header, buf);
}

void Connection::endheaders() {
    if (m_State != REQ_STARTED) {
        throw Wobbly("Cannot send header");
    }
    m_State = IDLE;

    m_Buffer.push_back("");

    string msg;
    for (vector<string>::const_iterator it = m_Buffer.begin(); it != m_Buffer.end(); ++it) {
        msg += (*it) + "\r\n";
    }

    m_Buffer.clear();
    send((const unsigned char*)msg.c_str(), static_cast<int>(msg.size()));
}

void Connection::send(const unsigned char* buf, int numbytes) {
    if (m_Sock < 0) {
        connect();
    }

    while (numbytes > 0) {
#ifdef _WIN32
        int n = ::send(static_cast<SOCKET>(m_Sock), (const char*)buf, numbytes, 0);
#else
        int n = ::send(m_Sock, buf, numbytes, 0);
#endif
        if (n < 0) {
            BailOnSocketError("send()");
        }
        numbytes -= n;
        buf += n;
    }
}

void Connection::pump() {
    if (m_Outstanding.empty()) {
        return;
    }

    assert(m_Sock > 0);

    if (!datawaiting(m_Sock)) {
        return;
    }

    unsigned char buf[2048];
    int a = recv(static_cast<SOCKET>(m_Sock), (char*)buf, sizeof(buf), 0);
    if (a < 0) {
        BailOnSocketError("recv()");
    }

    if (a == 0) {
        Response* r = m_Outstanding.front();
        r->notifyconnectionclosed();
        assert(r->completed());
        delete r;
        m_Outstanding.pop_front();
        close();
    } else {
        int used = 0;
        while (used < a && !m_Outstanding.empty()) {
            Response* r = m_Outstanding.front();
            int u = r->pump(&buf[used], a - used);
            if (r->completed()) {
                delete r;
                m_Outstanding.pop_front();
            }
            used += u;
        }
        assert(used == a);
    }
}

Response::Response(const char* method, Connection& conn) :
    m_State(STATUSLINE),
    m_Connection(conn),
    m_Method(method),
    m_Version(0),
    m_Status(0),
    m_BytesRead(0),
    m_Chunked(false),
    m_ChunkLeft(0),
    m_Length(-1),
    m_WillClose(false)
{
}

const char* Response::getheader(const char* name) const {
    std::string lname(name);
    std::transform(lname.begin(), lname.end(), lname.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::map<std::string, std::string>::const_iterator it = m_Headers.find(lname);
    if (it == m_Headers.end()) {
        return 0;
    }
    return it->second.c_str();
}

int Response::getstatus() const {
    assert(m_State != STATUSLINE);
    return m_Status;
}

const char* Response::getreason() const {
    assert(m_State != STATUSLINE);
    return m_Reason.c_str();
}

void Response::notifyconnectionclosed() {
    if (m_State == COMPLETE) {
        return;
    }

    if (m_State == BODY && !m_Chunked && m_Length == -1) {
        Finish();
    } else {
        throw Wobbly("Connection closed unexpectedly");
    }
}

int Response::pump(const unsigned char* data, int datasize) {
    assert(datasize != 0);
    int count = datasize;

    while (count > 0 && m_State != COMPLETE) {
        if (m_State == STATUSLINE ||
            m_State == HEADERS ||
            m_State == TRAILERS ||
            m_State == CHUNKLEN ||
            m_State == CHUNKEND)
        {
            while (count > 0) {
                char c = (char)*data++;
                --count;
                if (c == '\n') {
                    switch (m_State) {
                    case STATUSLINE:
                        ProcessStatusLine(m_LineBuf);
                        break;
                    case HEADERS:
                        ProcessHeaderLine(m_LineBuf);
                        break;
                    case TRAILERS:
                        ProcessTrailerLine(m_LineBuf);
                        break;
                    case CHUNKLEN:
                        ProcessChunkLenLine(m_LineBuf);
                        break;
                    case CHUNKEND:
                        assert(m_Chunked == true);
                        m_State = CHUNKLEN;
                        break;
                    default:
                        break;
                    }
                    m_LineBuf.clear();
                    break;
                } else {
                    if (c != '\r') {
                        m_LineBuf += c;
                    }
                }
            }
        } else if (m_State == BODY) {
            int bytesused = m_Chunked ? ProcessDataChunked(data, count) : ProcessDataNonChunked(data, count);
            data += bytesused;
            count -= bytesused;
        }
    }

    return datasize - count;
}

void Response::ProcessChunkLenLine(std::string const& line) {
    m_ChunkLeft = strtol(line.c_str(), NULL, 16);
    if (m_ChunkLeft == 0) {
        m_State = TRAILERS;
        m_HeaderAccum.clear();
    } else {
        m_State = BODY;
    }
}

int Response::ProcessDataChunked(const unsigned char* data, int count) {
    assert(m_Chunked);

    int n = count;
    if (n > m_ChunkLeft) {
        n = m_ChunkLeft;
    }

    if (m_Connection.m_ResponseDataCB) {
        (m_Connection.m_ResponseDataCB)(this, m_Connection.m_UserData, data, n);
    }

    m_BytesRead += n;
    m_ChunkLeft -= n;
    assert(m_ChunkLeft >= 0);
    if (m_ChunkLeft == 0) {
        m_State = CHUNKEND;
    }
    return n;
}

int Response::ProcessDataNonChunked(const unsigned char* data, int count) {
    int n = count;
    if (m_Length != -1) {
        int remaining = m_Length - m_BytesRead;
        if (n > remaining) {
            n = remaining;
        }
    }

    if (m_Connection.m_ResponseDataCB) {
        (m_Connection.m_ResponseDataCB)(this, m_Connection.m_UserData, data, n);
    }

    m_BytesRead += n;
    if (m_Length != -1 && m_BytesRead == m_Length) {
        Finish();
    }
    return n;
}

void Response::Finish() {
    m_State = COMPLETE;
    if (m_Connection.m_ResponseCompleteCB) {
        (m_Connection.m_ResponseCompleteCB)(this, m_Connection.m_UserData);
    }
}

void Response::ProcessStatusLine(std::string const& line) {
    const char* p = line.c_str();

    while (*p && *p == ' ') {
        ++p;
    }
    while (*p && *p != ' ') {
        m_VersionString += *p++;
    }
    while (*p && *p == ' ') {
        ++p;
    }

    std::string status;
    while (*p && *p != ' ') {
        status += *p++;
    }
    while (*p && *p == ' ') {
        ++p;
    }

    while (*p) {
        m_Reason += *p++;
    }

    m_Status = atoi(status.c_str());
    if (m_Status < 100 || m_Status > 999) {
        throw Wobbly("BadStatusLine (%s)", line.c_str());
    }

    if (m_VersionString == "HTTP:/1.0") {
        m_Version = 10;
    } else if (0 == m_VersionString.compare(0, 7, "HTTP/1.")) {
        m_Version = 11;
    } else {
        throw Wobbly("UnknownProtocol (%s)", m_VersionString.c_str());
    }

    m_State = HEADERS;
    m_HeaderAccum.clear();
}

void Response::FlushHeader() {
    if (m_HeaderAccum.empty()) {
        return;
    }

    const char* p = m_HeaderAccum.c_str();
    std::string header;
    std::string value;
    while (*p && *p != ':') {
        header += static_cast<char>(std::tolower(static_cast<unsigned char>(*p++)));
    }
    if (*p) {
        ++p;
    }
    while (*p && (*p == ' ' || *p == '\t')) {
        ++p;
    }
    value = p;
    m_Headers[header] = value;
    m_HeaderAccum.clear();
}

void Response::ProcessHeaderLine(std::string const& line) {
    const char* p = line.c_str();
    if (line.empty()) {
        FlushHeader();
        if (m_Status == CONTINUE) {
            m_State = STATUSLINE;
        } else {
            BeginBody();
        }
        return;
    }

    if (std::isspace(static_cast<unsigned char>(*p))) {
        ++p;
        while (*p && std::isspace(static_cast<unsigned char>(*p))) {
            ++p;
        }
        m_HeaderAccum += ' ';
        m_HeaderAccum += p;
    } else {
        FlushHeader();
        m_HeaderAccum = p;
    }
}

void Response::ProcessTrailerLine(std::string const& line) {
    if (line.empty()) {
        Finish();
    }
}

void Response::BeginBody() {
    m_Chunked = false;
    m_Length = -1;
    m_WillClose = false;

    const char* trenc = getheader("transfer-encoding");
    if (trenc && 0 == strcasecmp(trenc, "chunked")) {
        m_Chunked = true;
        m_ChunkLeft = -1;
    }

    m_WillClose = CheckClose();

    const char* contentlen = getheader("content-length");
    if (contentlen && !m_Chunked) {
        m_Length = atoi(contentlen);
    }

    if (m_Status == NO_CONTENT ||
        m_Status == NOT_MODIFIED ||
        (m_Status >= 100 && m_Status < 200) ||
        m_Method == "HEAD")
    {
        m_Length = 0;
    }

    if (!m_WillClose && !m_Chunked && m_Length == -1) {
        m_WillClose = true;
    }

    if (m_Connection.m_ResponseBeginCB) {
        (m_Connection.m_ResponseBeginCB)(this, m_Connection.m_UserData);
    }

    if (m_Chunked) {
        m_State = CHUNKLEN;
    } else {
        m_State = BODY;
    }
}

bool Response::CheckClose() {
    if (m_Version == 11) {
        const char* conn = getheader("connection");
        if (conn && 0 == strcasecmp(conn, "close")) {
            return true;
        }
        return false;
    }

    if (getheader("keep-alive")) {
        return false;
    }

    return true;
}

} // namespace happyhttp
