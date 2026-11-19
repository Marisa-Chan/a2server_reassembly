# Linux Dedicated Server Porting Analysis

The server is already 100% C++ (no MASM remaining) and already serves raw TCP/IP sockets (`PROVIDER_HAT_TCP`). Cutting out the client-only files (`vis_*.cpp`, `gfx.cpp`, `sound.cpp`, `mouse.cpp`, `smacker_player.cpp`, `res/a2server.rc`, etc.) and replacing the MFC `ServerControlDialog` / `MainWindow` UI removes most of the GUI/DirectX surface area.

Below is everything else required to build and run the dedicated server (`rea2s` / `rea2sp` / `rea2st`) natively on Linux.

---

## 1. Non-GUI MFC Layer (Largest Effort)

Even after stripping all client/GUI code, server logic relies heavily on non-GUI MFC classes:

- **Containers & value types:** `CString` (~875 refs, ~313 `.Format()` calls), `CArray` (~347), `CList` (~137), `CStringArray` (~114), `CMap` (~57), `CMapStringToString`, `CDWordArray`, `CPtrArray`, `CTime`, and `CPoint`/`CRect`/`CSize` (used in map/visibility/geometry code, not just UI).
- **File I/O:** `CFile`, `CStdioFile`, `CFileFind`. Note that `File2` inherits directly from `CFile` (`src/file.h`).
- **MFC RTTI & `CArchive` serialization:** `CObject`, `IMPLEMENT_DYNAMIC` / `IMPLEMENT_SERIAL`, `IsKindOf(RUNTIME_CLASS(...))`, and `CArchive` (`ReadObject`/`WriteObject`) are used across `Unit`, `Item`, `Inventory`, `Building`, `Inn`, `Effect`, `Sack`, `SpellEffect`, and `Server::SerializeGame` (`src/server.cpp`).
  - `IsKindOf()` is used at runtime (e.g., `src/buildings_list.cpp`, `src/gameobj.cpp`).
  - If the dedicated Linux server doesn't need `.a2s` save/load compatibility, `Serialize()` can be stubbed; otherwise a small `CArchive` + runtime-class registry shim is needed.

**Recommended approach:** Implement a lightweight header-only **MFC-compat shim** (`CString`, `CArray`, `CList`, `CMap`, `CFile`, `CPoint`/`CRect`, `CObject`/`RUNTIME_CLASS`) backed by `std::string`, `std::vector`, `std::list`, `std::unordered_map`, and `FILE*` rather than rewriting thousands of call sites across the shared server/client codebase.

---

## 2. Decoupling the Entry Point & Tick Loop from `CWinApp` / `MainWindow`

- Currently `src/cmain.cpp` invokes `AfxWinMain` -> `GameApp::InitInstance()` (`src/game_app.cpp`), which reads the Windows Registry (`HKLM\SOFTWARE\1C\Allods 2`), initializes DirectDraw (`InitVideo`/`SetVideoMode`), loads client `.res` archives/fonts/distort maps, creates `MainWindow`, and drives ticks through `GameApp::OnIdle()` -> `MainWindow::IdleLoop()` (`src/main_window.cpp`).
- Several server paths call `AfxGetMainWnd()` or `PostMessageA(g_MainWndHWND, MAIN_MSG_...)` (e.g., `PostConsoleLine` in `src/game_app.cpp`, `src/server.cpp`, `src/take_damage.cpp`).
- Extract the server-relevant fields from `MainWindow` (`game_speed`, `current_map_name`, `shutdown_pending`, log sink, control callbacks) into a platform-neutral `ServerHost` struct, drive `g_Server->ServerTic()` and `g_NetStru1_main.ProcessConnections()` from a plain `main()` loop, and attach any cross-platform control UI (TUI/CLI/web/ImGui) on top.

---

## 3. Networking: DirectPlay & Winsock -> POSIX Sockets

### DirectPlay vs TCP/IP
- **The dedicated server (`!A2CLIENT`) already uses raw TCP/IP exclusively — never DirectPlay:**
  - `MainWindow::Proc_421()` (`src/main_window.cpp`) posts `MAIN_MSG_START_HAT_DEDICATED` -> `MainWindow::StartHatDedicatedServer()`, which unconditionally sets `g_CLlDriver.ResetProvider(PROVIDER_HAT_TCP)` and calls `StartServerTcp()` (Winsock `socket`/`bind`/`listen`, default port `8000` or `:port` from `server.cfg`).
  - `Server::Start()` (`src/server.cpp`) also unconditionally sets `g_HatLLDriver.ResetProvider(PROVIDER_HAT_TCP)` for the HAT server connection.
  - `A2TANGAR_HAT` (`src/patch/hat_net.cpp`, `src/patch/hat_packet.cpp`) likewise uses plain TCP sockets directly.
- DirectPlay (`PROVIDER_SERIAL`, `PROVIDER_MODEM`, `PROVIDER_IPX`, `PROVIDER_DP_TCPIP`) is only used by client-hosted LAN/modem/serial games and the client LAN browser (`EnumConnections` / `EnterServerMode` / `EnterClientMode`).
- All COM/DirectPlay code (`dplay.h`, `CoCreateInstance`, `IDirectPlay3A`/`4A`) in `src/net.h` and `src/net.cpp` can be gated behind `#ifdef _WIN32` (or `#if A2_ENABLE_DPLAY`) with zero impact on dedicated server functionality.

### Winsock -> POSIX Socket Differences (`src/net.cpp`, `src/patch/hat_packet.cpp`)
- Drop `WSAStartup` / `WSACleanup`.
- Typedef `SOCKET` -> `int`, `INVALID_SOCKET` -> `-1`, `SOCKET_ERROR` -> `-1`.
- `closesocket()` -> `close()`.
- `ioctlsocket(fd, FIONBIO, &nonblock)` -> `fcntl(fd, F_SETFL, flags | O_NONBLOCK)`.
- `WSAGetLastError()` / `WSAEWOULDBLOCK` -> `errno` / `EAGAIN` or `EWOULDBLOCK`.
- **`select()` `nfds` bug:** `CLlDriver::AcceptThreadTcp` (`src/net.cpp:3078`) calls `select(0, &fd, NULL, NULL, &timeout)`. Windows ignores the first parameter, but on Linux `nfds` **must** be `max_fd + 1`, otherwise `select()` will always time out and never report readable sockets.
- **`SIGPIPE`:** Ignore `SIGPIPE` (`signal(SIGPIPE, SIG_IGN)` or `MSG_NOSIGNAL` on `send()`), otherwise writing to a dropped client socket kills the server process.

---

## 4. Filesystem Paths & Case Sensitivity

- Hardcoded backslashes appear across startup, resources, and patches (`"main\\text\\main.txt"`, `"World\\Data\\"`, `Config::ControlDirectory + "\\*"` in `src/game_app.cpp`, `src/resource.cpp`, `src/patch/ctl_handle.cpp`, `src/patch/locale.cpp`).
- **Case sensitivity:** Inside `.res` archives, `pathStrCpy()` (`src/util.cpp`) lowercases paths (`g_PathLower = 1`), so archive lookups are safe. Loose disk files (`server.cfg`, `maps/`, `World/Data/`, `ChrBase`, `ControlDirectory`) will fail on case-sensitive Linux filesystems if casing differs. Normalize `\` -> `/` and add case-insensitive disk lookup (or normalize the server data directory to lowercase).
- Replace Win32 filesystem APIs (`FindFirstFileA`/`FindNextFileA`, `GetLogicalDriveStringsA`, `GetModuleFileNameA`, `GetCurrentDirectoryA`/`SetCurrentDirectoryA`, `CreateDirectoryA`, `DeleteFileA`, `GetTempPathA`) with `std::filesystem` and `/proc/self/exe`.

---

## 5. Text Encoding (`CP866` <-> `CP1251` <-> `UTF-8`)

- Wire protocol, chat, `.txt` tables, and character saves use **CP866 (DOS)** / **Windows-1251**.
- Currently `src/game_app.cpp`, `src/main_window.cpp`, and `src/patch/ctl_handle.cpp` call Win32 `CharToOemA`, `OemToCharA`, and `CharToOemBuffA`.
- On Linux, replace these with fixed 256-byte CP1251 <-> CP866 lookup tables (which are also immune to host OS locale mismatches) and convert to UTF-8 only when rendering to a Linux terminal or control UI.
- Replace `GetSystemDefaultUILanguage()` in `src/patch/locale.cpp` with a config option or `$LANG`.

---

## 6. Subtle Runtime & Portability Traps

### `rand()` / `RAND_MAX` Mismatch (Silent Gameplay/Crypto Bug)
- MSVC's `RAND_MAX` is `32767` (`0x7FFF`), while glibc's on Linux is `2147483647` (`0x7FFFFFFF`).
- Server code hardcodes `32767` / `0x8000` in `GetRandS16`, `Rand01`, `Random0N` (`src/game_app.cpp:366`, `1085-1094`) and `src/inn.cpp:695` (also affecting `src/player_file.cpp` encryption keys).
- Calling glibc `rand()` directly will overflow and corrupt rolls/quests/saves.
- **Fix:** Use a dedicated MSVC-compatible 15-bit LCG (`seed = seed * 214013 + 2531011; return (seed >> 16) & 0x7FFF;`).

### 32-bit vs 64-bit (`ASSERT_SIZE`, `long`, Pointer Widths)
- There are 509 `ASSERT_SIZE` / `ASSERT_OFFSET` static assertions (`src/assert_offset.h`) targeting 32-bit MSVC struct layouts, plus `_USE_32BIT_TIME_T` in `src/CMakeLists.txt`.
- On Linux x86_64 (LP64), `long` is 64-bit (32-bit on Windows), and places like `HashKey` (`src/mfc_templ_impl.h:81`) cast pointers through `DWORD`.
- **Fastest path:** Compile 32-bit (`-m32`) on x86_64 Linux first; or gate `ASSERT_SIZE`/`ASSERT_OFFSET` behind `#ifdef _WIN32` and audit raw struct I/O and pointer casts for native 64-bit / ARM64 builds.

### Threading & Timing
- `CRITICAL_SECTION` (~55 uses in `src/net.cpp`) is **recursive** on Windows -> map to `std::recursive_mutex` (not `std::mutex`, to avoid deadlocks).
- `CreateThread` / `_beginthread`, `CreateEvent`, `WaitForSingleObject` (`src/net.cpp`, `src/patch/ctl_handle.cpp`) -> `std::thread` + `std::condition_variable`.
- `GetTickCount()` / `timeGetTime()` -> `clock_gettime(CLOCK_MONOTONIC)` returning `uint32_t` ms.
- `QueryPerformanceCounter` / `QueryPerformanceFrequency` (`src/perf.cpp`) -> `std::chrono::steady_clock`.
- `Sleep(ms)` -> `std::this_thread::sleep_for`.

### Crash Handling & SEH
- `__try` / `__except`, `SetUnhandledExceptionFilter`, and `dbghelp` (`StackWalk64`) in `src/cmain.cpp` and `src/patch/crash_filter.cpp` are MSVC/Win32-only -> replace with `SIGSEGV`/`SIGABRT` signal handlers + `backtrace()` or standard Linux core dumps.

### MSVC-isms & CMake
- Macro out `__cdecl` (~160), `__stdcall` (~48), `__thiscall`, and alias `_stricmp`/`stricmp` -> `strcasecmp`, `_strnicmp` -> `strncasecmp`, `_snprintf` -> `snprintf`, `_strdup` -> `strdup`, `_itoa`/`fopen_s`/`sprintf_s`.
- Split `src/CMakeLists.txt` into a minimal `A2_SERVER_CORE_SOURCES` list (excluding client/GUI TUs and `res/a2server.rc`) and guard `CMAKE_MFC_FLAG`, MSVC flags, and Win32 libraries behind `if(WIN32)` / `if(MSVC)`.

---

## Suggested Order of Work

1. **Headless Windows server first:** Split `A2_SERVER_CORE_SOURCES` in CMake and build a headless Windows target without `vis_*`, `gfx`, `sound`, `MainWindow`, or DirectPlay; verify clients can connect and play.
2. **Platform & MFC shim layer:** Introduce a `platform/` compatibility layer (sockets, recursive mutex/threads, monotonic clock, paths, CP1251<->CP866, 15-bit `a2_rand`) and the non-GUI MFC shim.
3. **Linux build & control UI:** Compile on Linux (starting with `-m32` or gating layout asserts for x86_64) and attach a lightweight cross-platform control UI (CLI/TUI/web/ImGui) on top of `ServerHost`.
