#include "table.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

#include "game_app.h"
#include "item.h"


IMPLEMENT_SERIAL(TableLine, CObject, 1); // 6362b0

// 512B33
TableLine::TableLine() {
}

// 513A4F
void TableLine::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->name;
    } else {
        ar >> this->name;
    }
    this->values.Serialize(ar);
}

// 512C4D
void TableLine::VMethod1(CString line, int32_t values_count, double* out_values) {
    CString sanitized;
    sanitized.Empty();

    for (int32_t i = 0; i < line.GetLength(); i++) {
        const char ch = line[i];
        if (ch != '"') {
            sanitized += ch;
        }
    }

    line = sanitized;
    line += '\t';

    this->values.SetSize(values_count);

    int32_t value_index = 0;
    int32_t tab_index = line.Find('\t');
    while (tab_index >= 0) {
        if (value_index == 0) {
            this->name = line.Left(tab_index);
        } else if (out_values != nullptr) {
            CString token = line.Left(tab_index);
            this->VMethod2(token, value_index, nullptr, &out_values[value_index - 1]);
        } else if (tab_index > 0) {
            CString token = line.Left(tab_index);
            int32_t* dst = &this->values[value_index - 1];
            this->VMethod2(token, this->values[value_index - 1], dst, nullptr);
        } else {
            this->values[value_index - 1] = -1;
        }

        line = line.Mid(tab_index + 1);
        tab_index = line.Find('\t');
        value_index++;
        if (value_index > values_count) {
            break;
        }
    }
}

// 5131C1
void TableLine::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    if (type_id == 0) {
        this->name = str;
        return;
    }

    if (out_double != nullptr) {
        *out_double = atof(str);
        return;
    }

    if (out_value == nullptr) {
        return;
    }

    if (str.GetLength() > 0) {
        *out_value = atoi(str);
    } else {
        *out_value = -1;
    }
}

// Inlined in ASM.
uint32_t AlphaMask(CString str) {
    str.TrimLeft();
    str.TrimRight();
    CString compact;
    for (int32_t i = 0; i < str.GetLength(); i++) {
        if (str[i] != ' ') {
            compact += str[i];
        }
    }
    str = compact;

    uint32_t mask = 0;
    int32_t length = str.GetLength();
    if (length > 31) {
        length = 31;
    }

    for (int32_t i = 1; i <= length; i++) {
        if (std::isalpha(str[i - 1])) {
            mask |= (1u << (i & 31));
        }
    }

    return mask;
}

// 571E50
WorldEquip::WorldEquip() {
    for (int32_t i = 0; i < 7; i++) {
        this->shape_material_matrix[i] = 0;
    }
}

// 5724b0
WorldEquip::~WorldEquip() {
}

// 513AA3
void WorldEquip::Serialize(CArchive& ar) {
    TableLine::Serialize(ar);

    if (ar.IsStoring()) {
        ar.Write(this->shape_material_matrix.data(), this->shape_material_matrix.size() * sizeof(uint16_t));
    } else {
        ar.Read(this->shape_material_matrix.data(), this->shape_material_matrix.size() * sizeof(uint16_t));
    }

    this->string_array.Serialize(ar);
}

// 513268
void WorldEquip::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    if (type_id == 1) {
        *out_value = 0;
    } else if (type_id == 2) {
        *out_value = 0;
    } else if (type_id == 6) {
        if (str == "blade") {
            *out_value = 1;
        } else if (str == "axe") {
            *out_value = 2;
        } else if (str == "crush") {
            *out_value = 3;
        } else if (str == "pike") {
            *out_value = 4;
        } else if (str == "shoot") {
            *out_value = 5;
        } else if (str == "fire") {
            *out_value = 11;
        } else if (str == "water") {
            *out_value = 12;
        } else if (str == "air") {
            *out_value = 13;
        } else if (str == "earth") {
            *out_value = 14;
        } else if (str == "astral") {
            *out_value = 15;
        } else {
            *out_value = 0;
        }
    } else {
        TableLine::VMethod2(str, type_id, out_value, out_double);
    }
}

// 572740
MagicItem::MagicItem() {
}

// 5728F0
MagicItem::~MagicItem() {
}

// 513C13
void MagicItem::Serialize(CArchive& ar) {
    TableLine::Serialize(ar);

    if (ar.IsStoring()) {
        ar.Write(this->shape_material_matrix.data(), 1); // WAT: writing only the first byte? Looks like a typo, but ok.
        ar << this->effect;
    } else {
        ar.Read(this->shape_material_matrix.data(), 1);
        ar >> this->effect;
    }
}

// 572850
void MagicItem::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    TableLine::VMethod2(str, type_id, out_value, out_double);
}

// 572590
MatShape::MatShape() {
}

// 572670
MatShape::~MatShape() {
}

// 513B05
void MatShape::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->name;
        ar.Write(&this->data, sizeof(this->data));
    } else {
        ar >> this->name;
        ar.Read(&this->data, sizeof(this->data));
    }
}

// 5729E0
MagicInfo::MagicInfo() {
}

// 572AC0
MagicInfo::~MagicInfo() {
}

// 572B90
MonsterInfo::MonsterInfo() {
    this->equipped_items.SetSize(2, -1);
}

// 572CA0
MonsterInfo::~MonsterInfo() {
}

// 513C82
void MonsterInfo::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    if (out_value == nullptr) {
        TableLine::VMethod2(str, type_id, out_value, out_double);
        return;
    }

    switch (type_id) {
    case 0xe:
        if (str.IsEmpty()) {
            *out_value = 0;
        } else if (str == "Poison") {
            *out_value = 1;
        } else if (str == "Cold") {
            *out_value = 2;
        } else if (str == "AP") {
            *out_value = 3;
        }
        break;
    case 0x31:
    case 0x33:
    case 0x35: {
        *out_value = 0;
        if (!str.IsEmpty()) {
            for (int32_t i = g_GameDataRes.spells.GetSize() - 1; i >= 1; i--) {
                if (g_GameDataRes.spells[i].name == str) {
                    *out_value = i;
                    break;
                }
            }
        }
        break;
    }
    case 0x39:
        *out_value = 0;
        if (str.GetLength() > 19) {
            *out_value = AlphaMask(str);
        }
        break;
    default:
        TableLine::VMethod2(str, type_id, out_value, out_double);
        break;
    }
}

// 513FF6
void MonsterInfo::Serialize(CArchive& ar) {
    TableLine::Serialize(ar);

    if (ar.IsStoring()) {
        for (int32_t i = 0; i < 2; i++) {
            ar << this->equipped_items[i];
        }
    } else {
        for (int32_t i = 0; i < 2; i++) {
            ar >> this->equipped_items[i];
        }
    }
}

// 572D80
HumanInfo::HumanInfo() {
    this->equipped_items.SetSize(10, -1);
}

// 572E90
HumanInfo::~HumanInfo() {
}

// 5143A6
void HumanInfo::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    if (type_id == 0x1A) {
        *out_value = 0;
        if (str.GetLength() > 19) {
            *out_value = AlphaMask(str);
        }
        return;
    }

    TableLine::VMethod2(str, type_id, out_value, out_double);
}

// 514A3A
void HumanInfo::Serialize(CArchive& ar) {
    TableLine::Serialize(ar);

    if (ar.IsStoring()) {
        for (int32_t i = 0; i < 10; i++) {
            ar << this->equipped_items[i];
        }
    } else {
        for (int32_t i = 0; i < 10; i++) {
            ar >> this->equipped_items[i];
        }
    }
}

// 572F70
BuildingInfo::BuildingInfo() {
}

// 573050
BuildingInfo::~BuildingInfo() {
}

// 514AC9
void BuildingInfo::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    if (5 <= type_id && type_id <= 6) {
        uint32_t mask = 0;
        int32_t length = str.GetLength();
        if (length > 32) {
            length = 32;
        }

        for (int32_t i = 0; i < length; i++) {
            if (str[i] == '1') {
                mask |= (1u << (i & 31));
            }
        }

        *out_value = mask;
        return;
    }

    TableLine::VMethod2(str, type_id, out_value, out_double);
}

// 514BCE
void BuildingInfo::Serialize(CArchive& ar) {
    TableLine::Serialize(ar);
}

// 573120
SpellInfo::SpellInfo() {
}

// 573210
SpellInfo::~SpellInfo() {
}

// 514F41
void SpellInfo::VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double) {
    switch (type_id) {
    case 0:
        this->name = str;
        return;
    case 3:
        if (str.IsEmpty()) {
            *out_value = 0;
        } else {
            switch (str[0]) {
            case 'A':
                *out_value = 3;
                break;
            case 'F':
                *out_value = 1;
                break;
            case 'G':
                *out_value = 4;
                break;
            case 'I':
                *out_value = 5;
                break;
            case 'W':
                *out_value = 2;
                break;
            default:
                *out_value = 0;
                break;
            }
        }
        break;
    case 5:
        *out_value = (_stricmp(str, "Unit") == 0) ? 1 : 2;
        break;
    case 6:
        *out_value = (_stricmp(str, "Spell Effect") == 0) ? 2 : 1;
        break;
    case 9:
        if (str == "Point") {
            *out_value = 1;
        } else if (str.Find("Round") > 0) {
            *out_value = 3;
        } else if (str.Find("Long") > 0) {
            *out_value = 4;
        } else if (str.Find("Phase") > 0) {
            *out_value = 5;
        } else {
            *out_value = 2;
        }
        break;
    case 0x0E:
        *out_value = (_stricmp(str, "Hang On Unit") == 0) ? 2 : 1;
        break;          
    default:
        if (out_double == nullptr) {
            if (str.IsEmpty()) {
                *out_value = -1;
            } else {
                *out_value = atoi(str);
            }
        } else {
            *out_double = atof(str);
        }
        break;
    }
}

// 51520F
void SpellInfo::Serialize(CArchive& ar) {
    TableLine::Serialize(ar);

    if (ar.IsStoring()) {
        ar << this->effect;
    } else {
        ar >> this->effect;
    }
}

// 510502
Item* GameDataRes::sub_510502(CString* name) {
    CString base_name;
    CString extra;
    this->sub_50DB4E(&extra, name, &base_name);

    uint8_t shape_id = this->sub_50D8BA(&base_name, &base_name);
    uint8_t material_id = this->sub_50DA04(&base_name, &base_name);

    CString stripped_name = base_name;
    this->sub_50DC69(material_id, &stripped_name);

    for (int32_t i = this->armors.GetSize() - 1; i >= 1; i--) {
        if (stripped_name == this->armors[i].name) {
            Item* item = new Armor(shape_id, material_id, (uint8_t)i);
            if (extra.GetLength() > 0) {
                item->sub_548F3F(extra);
            }
            return item;
        }
    }

    for (int32_t i = this->weapons.GetSize() - 1; i >= 1; i--) {
        if (base_name == this->weapons[i].name) {
            Item* item = new Weapon(shape_id, material_id, (uint8_t)i);
            if (extra.GetLength() > 0) {
                item->sub_548F3F(extra);
            }
            return item;
        }
    }

    int32_t shield_pos = base_name.Find(" Shield");
    if (shield_pos > 0) {
        stripped_name = base_name.Left(shield_pos);
    } else {
        stripped_name = base_name;
    }
    this->sub_50DC69(material_id, &stripped_name);

    for (int32_t i = this->shields.GetSize() - 1; i >= 1; i--) {
        if (stripped_name == this->shields[i].name) {
            Item* item = new Shield(shape_id, material_id, (uint8_t)i);
            if (extra.GetLength() > 0) {
                item->sub_548F3F(extra);
            }
            return item;
        }
    }

    for (int32_t i = this->magic_items.GetSize() - 1; i >= 1; i--) {
        if (base_name == this->magic_items[i].name) {
            Item* item = new Item(0xE, (uint8_t)i);
            if (extra.GetLength() > 0) {
                item->_effects.RemoveAll();
                item->sub_548F3F(extra);
            }
            return item;
        }
    }

    LogMessage("Invalid item " + *name);
    return nullptr;
}

// 50DF19
int GameDataRes::sub_50DF19(int experience) {
    int result = 0;
    int best_exp = 1;

    for (int i = 1; i < this->monsters.GetSize(); i++) {
        MonsterInfo& monster = this->monsters[i];

        // Skip fake entries?
        if (monster.name.GetLength() == 0) {
            continue;
        }

        MonsterInfoData* data = monster.Values().GetData();

        // type_id must be in range [0x40, 0x62]
        if (data->type_id < 0x40 || data->type_id > 0x62) {
            continue;
        }

        // Skip type_id 0x45 with face == 1 (Ghost.1)
        if (data->type_id == 0x45 && data->face == 1) {
            continue;
        }

        // Skip type_id 0x52 and 0x59 (F_Zombie.1 and F_Skeleton.1)
        if (data->type_id == 0x52 || data->type_id == 0x59) {
            continue;
        }

        // Skip level 5 mobs
        if (monster.name.Find(".5") != -1) {
            continue;
        }

        // Find monster with highest experience that's <= input experience
        if (data->experience <= experience && data->experience > best_exp) {
            result = i;
            best_exp = data->experience;
        }
    }

    return result;
}

int32_t GameDataRes::FUN_005125a8(int32_t id)
{ //5125a8
    for (int i = humans.GetSize() - 1; i > 0; i--)
    {
        if (humans[i].values.GetSize() && humans[i].Values()[0].server_id == id)
            return i;
    }
    return 0;
}

int32_t GameDataRes::FUN_00512625(int32_t id)
{ //512625
    for (int i = monsters.GetSize() - 1; i > 0; i--)
    {
        if (monsters[i].values.GetSize() && monsters[i].Values()[0].server_id == id)
            return i;
    }
    return 0;
}