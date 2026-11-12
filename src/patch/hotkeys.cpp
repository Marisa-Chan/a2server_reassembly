// Tangar custom hotkeys, ported from my-a2mgr hotkey.cpp (see hotkeys.h for the
// feature summary). The retail helpers the patch reached through raw pointers are
// called directly here: UserShortcut (the "Hotkey" struct), VisPlayingArea /
// MainWindow fields, and the parsed player-file sections.

#include "hotkeys.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <winsock2.h>
#include <windows.h>

#include "../game_app.h"
#include "../main_window.h"
#include "../player_file.h"
#include "../vis_playing_area.h"

namespace tangar_hotkeys {

namespace {

// Minimal byte-buffer stream (the used subset of my-a2mgr lib/serialize.hpp).
class BinaryStream {
public:
    bool LoadFromFile(const std::string& filename) {
        FILE* f = fopen(filename.c_str(), "rb");
        if (f == nullptr) {
            return false;
        }
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer.resize((size_t)len);
        if (len != 0 && fread(&buffer[0], 1, (size_t)len, f) != (size_t)len) {
            fclose(f);
            buffer.clear();
            return false;
        }
        fclose(f);
        pos = 0;
        return true;
    }

    bool SaveToFile(const std::string& filename) {
        FILE* f = fopen(filename.c_str(), "wb");
        if (f == nullptr) {
            return false;
        }
        bool ok = buffer.empty() || fwrite(&buffer[0], 1, buffer.size(), f) == buffer.size();
        fclose(f);
        return ok;
    }

    bool EndOfStream() {
        return pos >= buffer.size();
    }

    uint32_t ReadUInt32() {
        uint32_t v = 0;
        ReadData(&v, sizeof(v));
        return v;
    }

    int32_t ReadInt32() {
        return (int32_t)ReadUInt32();
    }

    void ReadData(void* where, uint32_t size) {
        uint32_t avail = (uint32_t)buffer.size() - pos;
        if (size > avail) {
            size = avail;
        }
        memcpy(where, &buffer[pos], size);
        pos += size;
    }

    void WriteUInt32(uint32_t v) {
        const uint8_t* p = (const uint8_t*)&v;
        buffer.insert(buffer.end(), p, p + sizeof(v));
    }

    void WriteData(const void* what, uint32_t size) {
        const uint8_t* p = (const uint8_t*)what;
        buffer.insert(buffer.end(), p, p + size);
    }

private:
    std::vector<uint8_t> buffer;
    uint32_t pos = 0;
};

std::unordered_map<int, std::string> CreateLabels() {
    std::unordered_map<int, std::string> result{
        {VK_BACK, "BACK"},
        {VK_TAB, "TAB"},
        {VK_INSERT, "INS"},
        {VK_DELETE, "DEL"},
        {VK_HOME, "HOME"},
        {VK_END, "END"},
        {VK_OEM_PLUS, "="},
        {VK_OEM_MINUS, "-"},
        {VK_OEM_COMMA, ","},
        {VK_OEM_PERIOD, "."},
        {VK_OEM_1, ";"},
        {VK_OEM_2, "/"},
        {VK_OEM_3, "~~"},
        {VK_OEM_4, "["},
        {VK_OEM_5, "\\"},
        {VK_OEM_6, "]"},
        {VK_OEM_7, "\""},
        {VK_MULTIPLY, "NUM *"},
        {VK_ADD, "NUM +"},
        {VK_SUBTRACT, "NUM -"},
        {VK_DECIMAL, "NUM ."},
        {VK_DIVIDE, "NUM /"},
    };

    for (int key = VK_F1; key <= VK_F24; ++key) {
        result[key] = "F" + std::to_string(key - VK_F1 + 1);
    }

    for (char key = 'A'; key <= 'Z'; ++key) {
        result[key] = std::string(1, key);
    }

    for (char key = VK_NUMPAD0; key <= VK_NUMPAD9; ++key) {
        result[key] = "NUM " + std::to_string(key - VK_NUMPAD0);
    }

    // Numbers are not supported on purpose: setting a number hotkey would need to
    // remove the corresponding control-group mapping first.

    return result;
}

const std::map<std::string, Action> alias_names = {
    {"help", Action::HELP},
    {"diplomacy", Action::DIPLOMACY},
    {"a", Action::UNIT_A},
    {"c", Action::UNIT_C},
    {"d", Action::UNIT_D},
    {"g", Action::UNIT_G},
    {"l", Action::UNIT_L},
    {"m", Action::UNIT_M},
    {"p", Action::UNIT_P},
    {"r", Action::UNIT_R},
    {"s", Action::UNIT_S},
    {"t", Action::UNIT_T},
    {"b", Action::ACTION_B},
    {"e", Action::ACTION_E},
    {"f", Action::ACTION_F},
    {"h", Action::ACTION_H},
    {"i", Action::ACTION_I},
    {"k", Action::ACTION_K},
    {"n", Action::ACTION_N},
    {"o", Action::ACTION_O},
    {"u", Action::ACTION_U},
    {"w", Action::ACTION_W},
};

int KeyFromName(const std::string& name) {
    if (name.length() == 1 && 'a' <= name[0] && name[0] <= 'z') {
        return toupper((unsigned char)name[0]);
    }
    if (name[0] == 'f') {
        int f_key = atoi(&name[1]);
        if (1 <= f_key && f_key <= 24) {
            return VK_F1 + f_key - 1;
        }
    } else if (name == "backspace") {
        return VK_BACK;
    } else if (name == "tab") {
        return VK_TAB;
    } else if (name == "ins" || name == "insert") {
        return VK_INSERT;
    } else if (name == "del" || name == "delete") {
        return VK_DELETE;
    } else if (name == "home") {
        return VK_HOME;
    } else if (name == "end") {
        return VK_END;
    } else if (name.length() == 4 && name.compare(0, 3, "num") == 0) {
        if ('0' <= name[3] && name[3] <= '9') {
            return VK_NUMPAD0 + name[3] - '0';
        }
        switch (name[3]) {
            case '*':
                return VK_MULTIPLY;
            case '+':
                return VK_ADD;
            case '-':
                return VK_SUBTRACT;
            case '.':
                return VK_DECIMAL;
            case '/':
                return VK_DIVIDE;
        }
    } else if (name.length() == 1) {
        switch (name[0]) {
            case '=':
                return VK_OEM_PLUS;
            case '-':
                return VK_OEM_MINUS;
            case ',':
                return VK_OEM_COMMA;
            case '.':
                return VK_OEM_PERIOD;
            case ';':
            case ':':
                return VK_OEM_1;
            case '/':
                return VK_OEM_2;
            case '`':
            case '~':
                return VK_OEM_3;
            case '[':
            case '{':
                return VK_OEM_4;
            case '\\':
                return VK_OEM_5;
            case ']':
            case '}':
                return VK_OEM_6;
            case '"':
            case '\'':
                return VK_OEM_7;
            case '+':
                return VK_ADD; // `+` means `num+`
        }
    }
    return 0;
}

bool LoadHotkeysFile(BinaryStream& stream, std::unordered_map<int, std::unique_ptr<UserShortcut>>& new_hotkeys) {
    if (stream.ReadUInt32() != 0xCC795074) {
        return false;
    }

    uint32_t total_hotkeys = stream.ReadUInt32();

    // 2560 bytes is overkill (a shortcut is 8 bytes + payload, at most 13 magic
    // properties), but the retail client allocates this much, so keep it.
    uint8_t buffer[2560];

    for (uint32_t i = 0; i < total_hotkeys; ++i) {
        if (stream.EndOfStream()) {
            return false;
        }

        int32_t key = stream.ReadInt32();
        uint32_t size = stream.ReadUInt32();
        stream.ReadData(buffer, size);

        new_hotkeys[key].reset(new UserShortcut());
        uint8_t* buf = buffer;
        new_hotkeys[key]->LoadFromBuffer(&buf);
    }

    return true;
}

} // namespace

std::unordered_map<int, std::string> labels = CreateLabels();
std::map<int, Action> aliases;
std::unordered_map<int, std::unique_ptr<UserShortcut>> custom_hotkeys;
uint32_t custom_hotkeys_loaded_for = 0;

// The patch remembered the objects seen at its hook entry points; the typed
// equivalents are kept the same way (set from OnKeyDown entry / panel toggle).
MainWindow* window_object = nullptr;
VisPlayingArea* key_handler_object = nullptr;
VisInvType1* panel_object = nullptr;

// No clean way to check the spell book, so track whether the open or closed book
// was rendered (is_spell_book_open hook).
bool spell_book_is_open = false;

void ParseAlias(const std::string& name, const std::string& action) {
    if (name.empty()) {
        return;
    }

    int key = KeyFromName(name);
    if (key == 0) {
        return;
    }

    auto it = alias_names.find(action);
    if (it == alias_names.end()) {
        return;
    }
    aliases[key] = it->second;
}

void LoadCustomHotkeys() {
    uint32_t id1 = GetParsedPlayerBasicInfo()->id1;
    if (custom_hotkeys_loaded_for == id1) {
        return;
    }

    BinaryStream stream;
    char filename[64];
    sprintf(filename, "hotkey.%u.a2key", id1);

    if (!stream.LoadFromFile(filename)) {
        // File doesn't exist, so no hotkeys are defined.
        custom_hotkeys.clear();
        return;
    }

    std::unordered_map<int, std::unique_ptr<UserShortcut>> new_hotkeys;
    if (!LoadHotkeysFile(stream, new_hotkeys)) {
        LogMessage(CString("Failed to load hotkeys!"));
        return;
    }

    custom_hotkeys = std::move(new_hotkeys);
    custom_hotkeys_loaded_for = id1;
}

void SaveCustomHotkeys() {
    // Signature (4), hotkey count (4), then per hotkey: key (4), serialized
    // shortcut size (4), serialized shortcut bytes (retail UserShortcut format).
    BinaryStream stream;

    stream.WriteUInt32(0xCC795074);
    stream.WriteUInt32((uint32_t)custom_hotkeys.size());

    uint8_t buffer[2560];

    for (auto it = custom_hotkeys.begin(); it != custom_hotkeys.end(); ++it) {
        stream.WriteUInt32((uint32_t)it->first);
        uint8_t* buf = buffer;
        uint32_t size = (uint32_t)it->second->ToBuffer(&buf);
        stream.WriteUInt32(size);
        stream.WriteData(buffer, size);
    }

    char filename[64];
    sprintf(filename, "hotkey.%u.a2key", GetParsedPlayerBasicInfo()->id1);

    if (!stream.SaveToFile(filename)) {
        LogMessage(CString("Failed to save hotkeys!"));
    }
}

const char* Label(int key) {
    auto it = labels.find(key);
    if (it != labels.end()) {
        return it->second.c_str();
    }
    return "";
}

void RememberKeyHandler(void* obj) {
    key_handler_object = (VisPlayingArea*)obj;
}

void RememberWindow(void* window) {
    window_object = (MainWindow*)window;
}

void SetSpellBookOpen(bool open) {
    spell_book_is_open = open;
}

bool SpellBookOpen() {
    return spell_book_is_open;
}

bool IsTyping() {
    return window_object != nullptr && window_object->console_active != 0;
}

bool AllowedWhileTyping(int key) {
    return (VK_F1 <= key && key <= VK_F24) || key == VK_INSERT || key == VK_DELETE || key == VK_HOME || key == VK_END;
}

UserShortcut* LookupHotkey(int key) {
    if (labels.count(key) == 0) {
        return nullptr;
    }

    if (window_object == nullptr) {
        return nullptr;
    }

    if (IsTyping() && !AllowedWhileTyping(key)) {
        return nullptr;
    }

    LoadCustomHotkeys();

    auto it = custom_hotkeys.find(key);
    if (it != custom_hotkeys.end() && it->second->kind != 0) {
        return it->second.get();
    }

    return nullptr;
}

bool HotkeysAvailable() {
    if (window_object == nullptr || key_handler_object == nullptr) {
        return false;
    }

    return window_object->dialogsMask == DIALOG_MASK_INGAME && key_handler_object->scenario != nullptr;
}

} // namespace tangar_hotkeys
