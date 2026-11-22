#if defined(A2SERVER_PATCH) && defined(A2TANGAR_HAT) && !defined(A2CLIENT)

#include "patch/server_icon.h"

#include <shobjidl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

namespace server_icon {

namespace {

constexpr WORD kServerIconResourceId = 109; // Icon_1.ico in res/a2server.rc

std::string TrimAscii(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string ToLowerAscii(std::string str) {
    for (char& ch : str) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return str;
}

std::string Unquote(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

void PeekServerIdFromFile(const std::string& filename,
                          std::vector<std::string>& visited,
                          uint32_t& out_server_id) {
    std::ifstream file(filename.c_str(), std::ios::in);
    if (!file.is_open()) {
        return;
    }

    std::string section = "root";
    std::string line;
    while (std::getline(file, line)) {
        bool in_quotes = false;
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '"') {
                in_quotes = !in_quotes;
            }
            if (in_quotes) {
                continue;
            }
            if (line[i] == '#' || line[i] == ';' ||
                (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/')) {
                line.erase(i);
                break;
            }
        }

        line = TrimAscii(line);
        if (line.empty()) {
            continue;
        }

        if (line.rfind("!include ", 0) == 0) {
            std::string inc = Unquote(TrimAscii(line.substr(9)));
            if (!inc.empty() &&
                std::find(visited.begin(), visited.end(), inc) == visited.end()) {
                visited.push_back(inc);
                PeekServerIdFromFile(inc, visited, out_server_id);
            }
            continue;
        }

        if (line.front() == '[') {
            size_t close = line.find(']');
            if (close == std::string::npos) {
                return;
            }
            section = ToLowerAscii(TrimAscii(line.substr(1, close - 1)));
            continue;
        }

        if (section != "settings") {
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = ToLowerAscii(TrimAscii(line.substr(0, eq)));
        if (key != "serverid") {
            continue;
        }

        std::string val = Unquote(TrimAscii(line.substr(eq + 1)));
        if (!val.empty()) {
            char* end = nullptr;
            unsigned long parsed = strtoul(val.c_str(), &end, 10);
            if (end != val.c_str() && *end == '\0') {
                out_server_id = static_cast<uint32_t>(parsed);
            }
        }
    }
}

uint32_t PeekServerId(const char* cmd_line) {
    std::string cfg_path = "server.cfg";
    if (cmd_line != nullptr) {
        const char* stmp = strstr(cmd_line, "-cfg\"");
        if (stmp != nullptr) {
            const char* qend = strchr(stmp + 5, '"');
            if (qend != nullptr && qend > stmp + 5) {
                cfg_path.assign(stmp + 5, static_cast<size_t>(qend - (stmp + 5)));
            }
        }
    }

    uint32_t server_id = 0;
    std::vector<std::string> visited;
    visited.push_back(cfg_path);
    PeekServerIdFromFile(cfg_path, visited, server_id);
    return server_id;
}

HBITMAP CreateTopDownDib32(HDC ref_dc, int w, int h, uint8_t** out_bits) {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hbm = CreateDIBSection(ref_dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (out_bits != nullptr) {
        *out_bits = static_cast<uint8_t*>(bits);
    }
    return hbm;
}

HICON CreateBadgedServerIcon(int w, int h, uint32_t server_id) {
    if (w <= 0 || h <= 0) {
        return nullptr;
    }

    HINSTANCE hinst = GetModuleHandleA(nullptr);
    HICON base_icon = static_cast<HICON>(LoadImageA(
        hinst, MAKEINTRESOURCEA(kServerIconResourceId), IMAGE_ICON, w, h, LR_DEFAULTCOLOR));
    if (base_icon == nullptr) {
        return nullptr;
    }

    HDC screen_dc = GetDC(nullptr);
    HDC mem_dc = CreateCompatibleDC(screen_dc);
    if (mem_dc == nullptr) {
        if (screen_dc != nullptr) {
            ReleaseDC(nullptr, screen_dc);
        }
        DestroyIcon(base_icon);
        return nullptr;
    }

    uint8_t* col_bits = nullptr;
    uint8_t* msk_bits = nullptr;
    HBITMAP hbm_col = CreateTopDownDib32(screen_dc, w, h, &col_bits);
    HBITMAP hbm_msk = CreateTopDownDib32(screen_dc, w, h, &msk_bits);
    if (hbm_col == nullptr || hbm_msk == nullptr || col_bits == nullptr || msk_bits == nullptr) {
        if (hbm_col != nullptr) DeleteObject(hbm_col);
        if (hbm_msk != nullptr) DeleteObject(hbm_msk);
        DeleteDC(mem_dc);
        if (screen_dc != nullptr) ReleaseDC(nullptr, screen_dc);
        DestroyIcon(base_icon);
        return nullptr;
    }

    memset(col_bits, 0, static_cast<size_t>(w * h * 4));
    memset(msk_bits, 0, static_cast<size_t>(w * h * 4));

    HGDIOBJ old_bmp = SelectObject(mem_dc, hbm_col);
    DrawIconEx(mem_dc, 0, 0, base_icon, w, h, 0, nullptr, DI_NORMAL);
    SelectObject(mem_dc, hbm_msk);
    DrawIconEx(mem_dc, 0, 0, base_icon, w, h, 0, nullptr, DI_MASK);

    // Normalize base icon alpha from the AND mask so palette icons get A=255.
    const int pixel_count = w * h;
    for (int i = 0; i < pixel_count; ++i) {
        uint8_t* px = col_bits + i * 4;
        bool mask_transparent = (msk_bits[i * 4] > 128) && (px[3] == 0);
        if (mask_transparent) {
            px[0] = px[1] = px[2] = px[3] = 0;
        } else if (px[3] == 0) {
            px[3] = 255;
        }
    }

    if (server_id > 0) {
        char label[16];
        snprintf(label, sizeof(label), "%u", server_id);
        const size_t digits = strlen(label);

        uint8_t* txt_bits = nullptr;
        HBITMAP hbm_txt = CreateTopDownDib32(screen_dc, w, h, &txt_bits);
        if (hbm_txt != nullptr && txt_bits != nullptr) {
            memset(txt_bits, 0, static_cast<size_t>(w * h * 4));
            SelectObject(mem_dc, hbm_txt);

            double scale = 0.78;
            int min_px = 11;
            if (digits == 2) {
                scale = 0.62;
                min_px = 10;
            } else if (digits >= 3) {
                scale = 0.48;
                min_px = 8;
            }
            int font_height = -(std::max)(min_px, static_cast<int>(std::lround(h * scale)));

            HFONT hfont = CreateFontA(
                font_height, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
            HGDIOBJ old_font = hfont != nullptr ? SelectObject(mem_dc, hfont) : nullptr;

            SetBkMode(mem_dc, TRANSPARENT);
            SetTextColor(mem_dc, RGB(255, 255, 255));
            RECT rc = {0, 0, w, h};
            DrawTextA(mem_dc, label, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

            if (old_font != nullptr) {
                SelectObject(mem_dc, old_font);
            }
            if (hfont != nullptr) {
                DeleteObject(hfont);
            }

            const int radius = (w < 24) ? 1 : 2;
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    const int idx = y * w + x;
                    const int cov = txt_bits[idx * 4]; // R==G==B under grayscale AA

                    int outline = 0;
                    for (int dy = -radius; dy <= radius; ++dy) {
                        int ny = y + dy;
                        if (ny < 0 || ny >= h) continue;
                        for (int dx = -radius; dx <= radius; ++dx) {
                            int nx = x + dx;
                            if (nx < 0 || nx >= w) continue;
                            int v = txt_bits[(ny * w + nx) * 4];
                            if (radius >= 2 && std::abs(dx) == radius && std::abs(dy) == radius) {
                                v = (v * 3) / 4;
                            }
                            if (v > outline) {
                                outline = v;
                            }
                        }
                    }

                    if (outline == 0 && cov == 0) {
                        continue;
                    }

                    const int out_a = (std::min)(255, (outline * 8) / 5);
                    uint8_t* dst = col_bits + idx * 4; // BGRA

                    // Composite black outline over base icon
                    const int inv_out = 255 - out_a;
                    int b = (dst[0] * inv_out) / 255;
                    int g = (dst[1] * inv_out) / 255;
                    int r = (dst[2] * inv_out) / 255;
                    int a = (std::max)(static_cast<int>(dst[3]), out_a);

                    // Composite white number glyph over outlined icon
                    const int inv_cov = 255 - cov;
                    dst[0] = static_cast<uint8_t>((b * inv_cov + 255 * cov) / 255);
                    dst[1] = static_cast<uint8_t>((g * inv_cov + 255 * cov) / 255);
                    dst[2] = static_cast<uint8_t>((r * inv_cov + 255 * cov) / 255);
                    dst[3] = static_cast<uint8_t>((std::max)(a, cov > 0 ? 255 : 0));
                }
            }
        }
        if (hbm_txt != nullptr) {
            SelectObject(mem_dc, old_bmp);
            DeleteObject(hbm_txt);
        } else {
            SelectObject(mem_dc, old_bmp);
        }
    } else {
        SelectObject(mem_dc, old_bmp);
    }

    // Build a matching 1-bit monochrome AND mask (WORD-aligned scanlines).
    const int mask_stride = ((w + 15) / 16) * 2;
    std::vector<uint8_t> mono_mask(static_cast<size_t>(mask_stride * h), 0);
    for (int y = 0; y < h; ++y) {
        uint8_t* row = mono_mask.data() + y * mask_stride;
        for (int x = 0; x < w; ++x) {
            if (col_bits[(y * w + x) * 4 + 3] == 0) {
                row[x >> 3] |= static_cast<uint8_t>(0x80u >> (x & 7));
            }
        }
    }
    HBITMAP hbm_mono = CreateBitmap(w, h, 1, 1, mono_mask.data());

    ICONINFO ii = {};
    ii.fIcon = TRUE;
    ii.hbmMask = hbm_mono;
    ii.hbmColor = hbm_col;
    HICON result_icon = CreateIconIndirect(&ii);

    if (hbm_mono != nullptr) DeleteObject(hbm_mono);
    DeleteObject(hbm_col);
    DeleteObject(hbm_msk);
    DeleteDC(mem_dc);
    if (screen_dc != nullptr) ReleaseDC(nullptr, screen_dc);
    DestroyIcon(base_icon);

    return result_icon;
}

} // namespace

uint32_t InitProcessIdentity(const char* cmd_line) {
    uint32_t server_id = PeekServerId(cmd_line);

    wchar_t app_id[64];
    if (server_id > 0) {
        swprintf_s(app_id, L"Allods2.Server.%u", server_id);
    } else {
        wcscpy_s(app_id, L"Allods2.Server");
    }
    SetCurrentProcessExplicitAppUserModelID(app_id);

    return server_id;
}

void Apply(HWND hwnd, uint32_t server_id) {
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        return;
    }

    static HWND s_last_hwnd = nullptr;
    static uint32_t s_last_server_id = UINT32_MAX;
    static HICON s_big_icon = nullptr;
    static HICON s_small_icon = nullptr;

    if (hwnd == s_last_hwnd &&
        server_id == s_last_server_id &&
        s_big_icon != nullptr &&
        s_small_icon != nullptr) {
        return;
    }

    int big_w = GetSystemMetrics(SM_CXICON);
    int big_h = GetSystemMetrics(SM_CYICON);
    if (big_w <= 0 || big_h <= 0) {
        big_w = big_h = 32;
    }

    int sm_w = GetSystemMetrics(SM_CXSMICON);
    int sm_h = GetSystemMetrics(SM_CYSMICON);
    if (sm_w <= 0 || sm_h <= 0) {
        sm_w = sm_h = 16;
    }

    HICON new_big = CreateBadgedServerIcon(big_w, big_h, server_id);
    HICON new_small = CreateBadgedServerIcon(sm_w, sm_h, server_id);
    if (new_big == nullptr || new_small == nullptr) {
        if (new_big != nullptr) DestroyIcon(new_big);
        if (new_small != nullptr) DestroyIcon(new_small);
        return;
    }

    SendMessageA(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(new_big));
    SendMessageA(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(new_small));
    SetClassLongPtrA(hwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(new_big));
    SetClassLongPtrA(hwnd, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(new_small));

    if (s_big_icon != nullptr) DestroyIcon(s_big_icon);
    if (s_small_icon != nullptr) DestroyIcon(s_small_icon);

    s_big_icon = new_big;
    s_small_icon = new_small;
    s_last_hwnd = hwnd;
    s_last_server_id = server_id;
}

} // namespace server_icon

#endif // defined(A2SERVER_PATCH) && defined(A2TANGAR_HAT) && !defined(A2CLIENT)
