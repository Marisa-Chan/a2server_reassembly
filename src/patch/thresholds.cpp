// Tangar thresholds configuration, ported from my-srvmgr thresholds.cpp. The
// only deviation: the character-kind helpers work through our Unit/Player types
// (name prefix + typeId + circle::Circle + pOwner->deaths).

#include "thresholds.h"

#include <fstream>
#include <sstream>
#include <stack>

#include "circle.h"
#include "tangar_log.h"

#include "../player.h"
#include "../unit.h"

namespace thresholds {

const char* const THRESHOLDS_FILE = "../thresholds.cfg";

Thresholds thresholds;

namespace {

std::string TrimStr(const std::string& s) {
    size_t a = 0;
    size_t b = s.size();
    while (a < b && isspace((unsigned char)s[a])) {
        a++;
    }
    while (b > a && isspace((unsigned char)s[b - 1])) {
        b--;
    }
    return s.substr(a, b - a);
}

std::vector<std::string> Explode(const std::string& what, const std::string& separator) {
    std::vector<std::string> out;
    std::string cur;
    for (size_t i = 0; i < what.length(); i++) {
        if (what.compare(i, separator.length(), separator) == 0) {
            out.push_back(cur);
            cur.clear();
            i += separator.length() - 1;
            continue;
        }
        cur += what[i];
    }
    out.push_back(cur);
    return out;
}

bool CheckIntStr(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!isdigit((unsigned char)c)) {
            return false;
        }
    }
    return true;
}

uint32_t StrToUInt(const std::string& s) {
    uint32_t v = 0;
    for (char c : s) {
        if (!isdigit((unsigned char)c)) {
            break;
        }
        v = v * 10 + (uint32_t)(c - '0');
    }
    return v;
}

// Solo-mode name prefixes (my-srvmgr solo.cpp): '@' ironman, '!' pure, '_' legend.
bool IsIronPlayer(const Unit* unit) {
    const char* name = unit->pOwner != nullptr ? (const char*)unit->pOwner->name : "";
    return name[0] == '@';
}

bool IsPurePlayer(const Unit* unit) {
    const char* name = unit->pOwner != nullptr ? (const char*)unit->pOwner->name : "";
    return name[0] == '!';
}

bool IsGigaPlayer(const Unit* unit) {
    const char* name = unit->pOwner != nullptr ? (const char*)unit->pOwner->name : "";
    return name[0] == '_';
}

bool IsFemale(const Unit* unit) {
    return unit->typeId == 34 || unit->typeId == 36;
}

ServerIDType ServerType(const std::string& value) {
    if (value == "EASY") {
        return EASY;
    } else if (value == "KIDS") {
        return KIDS;
    } else if (value == "NIVAL") {
        return NIVAL;
    } else if (value == "MEDIUM") {
        return MEDIUM;
    } else if (value == "HARD") {
        return HARD;
    } else if (value == "NIGHTMARE") {
        return NIGHTMARE;
    } else if (value == "QUEST_T1") {
        return QUEST_T1;
    } else if (value == "QUEST_T2") {
        return QUEST_T2;
    } else if (value == "QUEST_T3") {
        return QUEST_T3;
    } else if (value == "QUEST_T4") {
        return QUEST_T4;
    }
    return UNDEFINED;
}

} // namespace

void Thresholds::LoadFromContent(std::string content) {
    std::istringstream stream(content);
    LoadFromStream(stream);
}

void Thresholds::LoadFromFile(std::string file_name) {
    std::ifstream fin(file_name);
    if (!fin) {
        throw ParseException("failed to open file '" + file_name + "'");
    }

    LoadFromStream(fin);
}

void Thresholds::LoadFromStream(std::istream& stream) {
    std::string line;
    int line_number = 0;

    std::stack<std::shared_ptr<Node>> stack;
    auto root = std::make_shared<Node>();
    stack.push(root);

    while (std::getline(stream, line)) {
        ++line_number;

        size_t comment = line.find("//");
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }

        line = TrimStr(line);
        if (line.length() == 0) {
            continue;
        }

        if (line.back() == '{') {
            std::string section_name = TrimStr(line.substr(0, line.length() - 1));

            auto node = std::make_shared<Node>();
            for (auto alias : Explode(section_name, "|")) {
                alias = TrimStr(alias);

                auto server_id = ServerType(alias);
                if (server_id != UNDEFINED) {
                    stack.top()->by_server[server_id] = node;
                } else {
                    stack.top()->children[alias] = node;
                }

                if (!stack.top()->children.empty() && !stack.top()->by_server.empty()) {
                    throw ParseException("line " + std::to_string(line_number)
                        + ": section contains a mix of subsection types");
                }
            }
            stack.push(node);
        } else if (line.back() == '}') {
            if (stack.empty()) {
                throw ParseException("line " + std::to_string(line_number)
                    + ": invalid syntax: too many closing brackets");
            }

            stack.pop();
        } else {
            size_t equals = line.find('=');

            if (equals == std::string::npos) {
                throw ParseException("line " + std::to_string(line_number)
                    + ": invalid syntax: '" + line + "' is not a section or a key-value pair");
            }

            std::string key = TrimStr(line.substr(0, equals));

            // Parse the value.
            std::string value = TrimStr(line.substr(equals + 1));
            uint32_t multiplier = 1;

            if (value.back() == 'k') {
                multiplier = 1000;
                value.pop_back();
            } else if (value.back() == 'm') {
                multiplier = 1000 * 1000;
                value.pop_back();
            }

            if (!CheckIntStr(value)) {
                throw ParseException("line " + std::to_string(line_number)
                    + ": invalid syntax: '" + value + "' is not a number");
            }

            uint32_t number = StrToUInt(value) * multiplier;

            auto& node = stack.top();

            // Parse the key type. Can be a regular key-value mapping or a
            // server_id mapping, or a mob mapping.
            auto server = ServerType(key);
            if (server != UNDEFINED) {
                node->by_server[server] = std::make_shared<Node>(number);
            } else {
                if (CheckIntStr(key)) {
                    uint16_t mob_id = (uint16_t)StrToUInt(key);
                    node->mobs[mob_id] = (int32_t)number;
                } else {
                    node->children[key] = std::make_shared<Node>(number);
                }
            }

            if (node->children.empty() + node->by_server.empty() + node->mobs.empty() != 2) {
                throw ParseException("line " + std::to_string(line_number)
                    + ": section contains a mix of key types");
            }
        }
    }

    if (stack.size() != 1) {
        throw ParseException("line " + std::to_string(line_number)
            + ": invalid syntax: section was not closed");
    }

    this->root = std::move(root);
    this->loaded_at = std::chrono::steady_clock::now();
}

void Thresholds::MaybeReload(std::string file_name) {
    auto now = std::chrono::steady_clock::now();

    // Reload at most once a minute.
    if (!this->root || (now - this->loaded_at) > std::chrono::minutes(1)) {
        try {
            Printf("Reloading thresholds");
            LoadFromFile(file_name);
        } catch (const ParseException& error) {
            Printf("Failed to reload thresholds: %s", error.what());
        }
    }
}

uint32_t Thresholds::Value(std::string key, const Unit* main_unit) const {
    const Node* node = this->Descend(this->Path(key), main_unit);
    return node ? node->number : 0;
}

const std::unordered_map<uint16_t, int>* Thresholds::Mobs(std::string key, const Unit* main_unit) const {
    const Node* node = this->Descend(this->Path(key), main_unit);
    return node ? &node->mobs : nullptr;
}

const Node* Thresholds::Path(std::string key) const {
    Node* node = this->root.get();

    for (auto p : Explode(key, ".")) {
        node = node->children[p].get();
        if (!node) {
            return nullptr;
        }
    }

    return node;
}

const Node* Thresholds::Descend(const Node* at, const Unit* main_unit) const {
    if (!at) {
        return nullptr;
    }

    // First descend by character kind.
    if (!at->children.empty()) {
        if (IsFemale(main_unit)) {
            auto ptr = at->children.find("female");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (circle::Circle(main_unit) > 0) {
            auto ptr = at->children.find("hell");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (IsGigaPlayer(main_unit)) {
            auto ptr = at->children.find("legend");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (IsIronPlayer(main_unit)) {
            auto ptr = at->children.find("ironman");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (IsPurePlayer(main_unit)) {
            auto ptr = at->children.find("ironman");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        if (main_unit->pOwner != nullptr && main_unit->pOwner->deaths <= 1) {
            auto ptr = at->children.find("hardcore");
            if (ptr != at->children.end()) {
                return this->Descend(ptr->second.get(), main_unit);
            }
        }

        auto ptr = at->children.find("default");
        if (ptr != at->children.end()) {
            return this->Descend(ptr->second.get(), main_unit);
        }
    }

    // Then descend by server ID.
    if (!at->by_server.empty()) {
        auto ptr = at->by_server.find(Config::ServerID);
        if (ptr == at->by_server.end()) {
            // Intentional. If settings aren't filled for the server ID, there
            // are no thresholds.
            return nullptr;
        }

        return this->Descend(ptr->second.get(), main_unit);
    }

    return at;
}

} // namespace thresholds
