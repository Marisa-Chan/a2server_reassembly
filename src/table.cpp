#include "table.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

#include "game_app.h"
#include "file.h"
#include "item.h"
#include "resource.h"

extern "C" CStringArray unk_6B0BE8;
extern "C" CStringArray unk_6CDB10;
extern "C" CStringArray unk_6B1638;
extern "C" CStringArray unk_6CF3D8;
extern "C" CStringArray unk_6C37E8;
extern "C" CStringArray unk_6E18C8;
extern "C" CStringArray unk_6B0BD0;
extern "C" CStringArray unk_6B1620;


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

// 578040
void WorldEquip::ParseLine(CString line) {
    this->VMethod1(line, 0x11, nullptr);
}

// 513B68
void MagicItem::ParseLine(CString line) {
    this->VMethod1(line, 2, nullptr);
    int32_t tabIndex = line.ReverseFind('\t');
    this->effect = line.Mid(tabIndex + 1);
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

// 5780B0
void MatShape::ParseLine(CString line) {
    this->VMethod1(line, 9, (double*)&this->data);
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

// 578120
void MagicInfo::ParseLine(CString line) {
    this->VMethod1(line, 0x1c, nullptr);
}

// 5729E0
MagicInfo::MagicInfo() {
}

// 572AC0
MagicInfo::~MagicInfo() {
}

// 578190
void MonsterInfo::ParseLine(CString line) {
    this->VMethod1(line, 0x3e, nullptr);
    sub_514085(line, 2, &this->equipped_items);
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

// 578220
void HumanInfo::ParseLine(CString line) {
    this->VMethod1(line, 0x1a, nullptr);
    sub_514085(line, 10, &this->equipped_items);
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

// 5782B0
void BuildingInfo::ParseLine(CString line) {
    this->VMethod1(line, 6, nullptr);
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

// 578320
void SpellInfo::ParseLine(CString line) {
    this->VMethod1(line, 0x16, nullptr);
    int32_t tabIndex = line.ReverseFind('\t');
    this->effect = line.Mid(tabIndex + 1);
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

// 50E0F0
void GameDataRes::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        unk_6B1638.Serialize(ar);
        ar << this->shapes.GetSize();
        for (int32_t i = 0; i < this->shapes.GetSize(); i++) {
            this->shapes[i].Serialize(ar);
        }
        ar << this->materials.GetSize();
        for (int32_t i = 0; i < this->materials.GetSize(); i++) {
            this->materials[i].Serialize(ar);
        }

        unk_6CF3D8.Serialize(ar);
        ar << this->magics.GetSize();
        for (int32_t i = 0; i < this->magics.GetSize(); i++) {
            this->magics[i].Serialize(ar);
        }

        unk_6CDB10.Serialize(ar);
        ar << this->armors.GetSize();
        for (int32_t i = 1; i < this->armors.GetSize(); i++) {
            this->armors[i].Serialize(ar);
        }
        ar << this->shields.GetSize();
        for (int32_t i = 1; i < this->shields.GetSize(); i++) {
            this->shields[i].Serialize(ar);
        }
        ar << this->weapons.GetSize();
        for (int32_t i = 1; i < this->weapons.GetSize(); i++) {
            this->weapons[i].Serialize(ar);
        }

        unk_6C37E8.Serialize(ar);
        ar << this->magic_items.GetSize();
        for (int32_t i = 1; i < this->magic_items.GetSize(); i++) {
            this->magic_items[i].Serialize(ar);
        }

        unk_6E18C8.Serialize(ar);
        ar << this->monsters.GetSize();
        for (int32_t i = 1; i < this->monsters.GetSize(); i++) {
            this->monsters[i].Serialize(ar);
        }

        unk_6B0BD0.Serialize(ar);
        ar << this->humans.GetSize();
        for (int32_t i = 1; i < this->humans.GetSize(); i++) {
            this->humans[i].Serialize(ar);
        }

        unk_6B1620.Serialize(ar);
        ar << this->buildings.GetSize();
        for (int32_t i = 1; i < this->buildings.GetSize(); i++) {
            this->buildings[i].Serialize(ar);
        }

        unk_6B0BE8.Serialize(ar);
        ar << this->spells.GetSize();
        for (int32_t i = 1; i < this->spells.GetSize(); i++) {
            this->spells[i].Serialize(ar);
        }
    } else {
        unk_6B1638.Serialize(ar);
        int32_t count = 0;
        ar >> count;
        this->shapes.SetSize(count, -1);
        for (int32_t i = 0; i < this->shapes.GetSize(); i++) {
            this->shapes[i].Serialize(ar);
        }
        ar >> count;
        this->materials.SetSize(count, -1);
        for (int32_t i = 0; i < this->materials.GetSize(); i++) {
            this->materials[i].Serialize(ar);
        }

        unk_6CF3D8.Serialize(ar);
        ar >> count;
        this->magics.SetSize(count, -1);
        for (int32_t i = 0; i < this->magics.GetSize(); i++) {
            this->magics[i].Serialize(ar);
        }

        unk_6CDB10.Serialize(ar);
        ar >> count;
        this->armors.SetSize(count, -1);
        for (int32_t i = 1; i < this->armors.GetSize(); i++) {
            this->armors[i].Serialize(ar);
        }
        ar >> count;
        this->shields.SetSize(count, -1);
        for (int32_t i = 1; i < this->shields.GetSize(); i++) {
            this->shields[i].Serialize(ar);
        }
        ar >> count;
        this->weapons.SetSize(count, -1);
        for (int32_t i = 1; i < this->weapons.GetSize(); i++) {
            this->weapons[i].Serialize(ar);
        }

        unk_6C37E8.Serialize(ar);
        ar >> count;
        this->magic_items.SetSize(count, -1);
        for (int32_t i = 1; i < this->magic_items.GetSize(); i++) {
            this->magic_items[i].Serialize(ar);
        }

        unk_6E18C8.Serialize(ar);
        ar >> count;
        this->monsters.SetSize(count, -1);
        for (int32_t i = 1; i < this->monsters.GetSize(); i++) {
            this->monsters[i].Serialize(ar);
        }

        unk_6B0BD0.Serialize(ar);
        ar >> count;
        this->humans.SetSize(count, -1);
        for (int32_t i = 1; i < this->humans.GetSize(); i++) {
            this->humans[i].Serialize(ar);
        }

        unk_6B1620.Serialize(ar);
        ar >> count;
        this->buildings.SetSize(count, -1);
        for (int32_t i = 1; i < this->buildings.GetSize(); i++) {
            this->buildings[i].Serialize(ar);
        }

        unk_6B0BE8.Serialize(ar);
        ar >> count;
        this->spells.SetSize(count, -1);
        for (int32_t i = 1; i < this->spells.GetSize(); i++) {
            this->spells[i].Serialize(ar);
        }
    }
}

// 50D8BA
uint8_t GameDataRes::sub_50D8BA(CString* name, CString* out) {
    for (int32_t i = this->shapes.GetSize() - 1; i >= 0; i--) {
        int32_t pos = name->Find(this->shapes[i].name);
        if (pos != -1) {
            if (out != nullptr) {
                *out = name->Left(pos) + name->Mid(this->shapes[i].name.GetLength() + 1);
            }
            return static_cast<uint8_t>(i);
        }
    }

    if (out != nullptr) {
        *out = *name;
    }
    return 0;
}

// 50DA04
uint8_t GameDataRes::sub_50DA04(CString* name, CString* out) {
    for (int32_t i = this->materials.GetSize() - 1; i >= 0; i--) {
        int32_t pos = name->Find(this->materials[i].name);
        if (pos != -1) {
            if (out != nullptr) {
                *out = name->Left(pos) + name->Mid(this->materials[i].name.GetLength() + 1);
            }
            return static_cast<uint8_t>(i);
        }
    }

    if (out != nullptr) {
        *out = *name;
    }
    return 0x0F;
}

// 50DB4E
CString* GameDataRes::sub_50DB4E(CString* out1, const CString* in_name, CString* base_name_out) {
    int32_t brace_open = in_name->Find('{');
    if (brace_open == -1) {
        *base_name_out = *in_name;
        *out1 = "";
        return out1;
    }

    *base_name_out = in_name->Left(brace_open);
    base_name_out->TrimRight();

    int32_t brace_close = in_name->Find('}');
    if (brace_close == -1) {
        brace_close = in_name->GetLength();
    }

    CString temp = in_name->Left(brace_close);
    *out1 = temp.Mid(brace_open + 1);
    return out1;
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

// 510F88
Item* GameDataRes::sub_510F88(uint8_t slot, uint8_t shape_id, uint8_t material_id, uint8_t item_data_id) {
    if (slot == 1) {
        return new Weapon(shape_id, material_id, item_data_id);
    }
    if (slot == 2) {
        return new Shield(shape_id, material_id, item_data_id);
    }
    if (slot >= 3 && slot <= 0x0D) {
        return new Armor(shape_id, material_id, item_data_id);
    }
    if (slot == 0x0E) {
        return new Item(0x0E, item_data_id);
    }
    return nullptr;
}

// 510EE6
Item* GameDataRes::sub_510EE6(uint16_t item_id) {
    uint8_t slot = (item_id >> 8) & 0x0F;

    if (slot == 0x0E) {
        return this->sub_510F88(slot, 0, 0, item_id);
    }

    uint8_t shape_id = (item_id >> 5) & 0x07;
    uint8_t material_id = (item_id >> 12) & 0x0F;
    uint8_t item_data_id = item_id & 0x1F;

    return this->sub_510F88(slot, shape_id, material_id, item_data_id);
}

// 50DC69
void GameDataRes::sub_50DC69(int material_id, CString* name) {
    if (this->materials[material_id].name.Find("Leather") != -1) {
        name->TrimLeft();
        *name = "Soft " + *name;
    } else if (this->materials[material_id].name.Find("Wood") != -1) {
        name->TrimLeft();
        *name = "Wooden " + *name;
    }
}

// 50DDAE
int GameDataRes::sub_50DDAE(uint8_t type_id, uint8_t face) {
    if (type_id < 0x40 && type_id != 0x1A && type_id != 0x1B) {
        for (int32_t i = 1; i < this->humans.GetSize(); i++) {
            if (this->humans[i].name.GetLength() != 0 && this->humans[i].Values()[0].type_id == type_id) {
                return i;
            }
        }
    } else {
        for (int32_t i = 1; i < this->monsters.GetSize(); i++) {
            if (this->monsters[i].name.GetLength() != 0 && this->monsters[i].Values()[0].type_id == type_id && this->monsters[i].Values()[0].face == face) {
                return i;
            }
        }
    }

    return 0;
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

// 5126A2
int GameDataRes::sub_5126A2(uint8_t type, uint8_t face) {
    for (int32_t i = this->monsters.GetSize() - 1; i > 0; i--) {
        if (this->monsters[i].values.GetSize() == 0) {
            continue;
        }

        MonsterInfoData* data = this->monsters[i].Values().GetData();
        if (data->type_id == type && data->face == face) {
            return i;
        }
    }

    return 0;
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

// 514BE7
void GameDataRes::sub_514BE7() {
    CArray<int32_t>& base_values = this->magics[0].values;
    base_values.SetSize(28, -1);

    for (int32_t i = 4; i < 28; i++) {
        base_values[i] = 0;
    }

    int32_t magic_count = this->magics.GetSize();
    for (int32_t col = 4; col < 28; col++) {
        for (int32_t row = 1; row < magic_count; row++) {
            int32_t& value = this->magics[row].values[col];
            if (value < 0) {
                value = 0;
            }
            value += this->magics[row - 1].values[col];
        }
    }
}

// 50a26b
int OpenText(const CString& path, CStdioFile& file) {
    BOOL success = file.Open(path, CFile::modeRead | CFile::typeText | CFile::shareDenyWrite);
    if (!success) {
        LogMessage("Error - file " + path + " not found");
    }
    return success;
}

// 50a345
int GameDataRes::ParseTxtFiles(const CString& path) {
    CString line;
    CStdioFile file;

    if (!OpenText(path + "Spells.txt", file)) {
        return 1;
    }
    this->spells.SetSize(35, -1);

    // Vanilla code populates these from global strings via `sub_5138ED`, and strings aren't written to, so it seems they're empty.
    unk_6B0BE8.SetSize(0, -1);
    unk_6B0BE8.SetAtGrow(0, "");

    int32_t index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (line.Find("rem") == 0) {
            index++;
            continue;
        }
        if (this->spells.GetSize() <= index) {
            this->spells.SetSize(index + 1, -1);
        }
        this->spells[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Armors.txt", file)) {
        return 1;
    }
    this->armors.SetSize(15, -1);
    unk_6CDB10.SetSize(0, -1);
    unk_6CDB10.SetAtGrow(0, "");
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (this->armors.GetSize() <= index) {
            this->armors.SetSize(index + 1, -1);
        }
        this->armors[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Materials.txt", file)) {
        return 1;
    }
    this->materials.SetSize(16, -1);
    unk_6B1638.SetSize(0, -1);
    unk_6B1638.SetAtGrow(0, "");
    index = 0;
    while (file.ReadString(line)) {
        if (line.GetLength() < 12 || line[0] == '\t') {
            continue;
        }
        if (this->materials.GetSize() <= index) {
            this->materials.SetSize(index + 1, -1);
        }
        this->materials[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Shapes.txt", file)) {
        return 1;
    }
    this->shapes.SetSize(7, -1);
    unk_6CF3D8.SetSize(0, -1);
    unk_6CF3D8.SetAtGrow(0, "");
    index = 0;
    while (file.ReadString(line)) {
        if (line.GetLength() < 12 || line[0] == '\t') {
            continue;
        }
        if (this->shapes.GetSize() <= index) {
            this->shapes.SetSize(index + 1, -1);
        }
        this->shapes[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Magic.txt", file)) {
        return 1;
    }
    this->magics.SetSize(24, -1);
    unk_6C37E8.SetSize(0, -1);
    unk_6C37E8.SetAtGrow(0, "");
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (line.Find("rem") == 0) {
            index++;
            continue;
        }
        if (this->magics.GetSize() <= index) {
            this->magics.SetSize(index + 1, -1);
        }
        this->magics[index].ParseLine(line);
        index++;
    }
    file.Close();
    this->sub_514BE7();

    if (!OpenText(path + "Weapons.txt", file)) {
        return 1;
    }
    this->weapons.SetSize(20, -1);
    unk_6E18C8.SetSize(0, -1);
    unk_6E18C8.SetAtGrow(0, "");
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (this->weapons.GetSize() <= index) {
            this->weapons.SetSize(index + 1, -1);
        }
        this->weapons[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Shields.txt", file)) {
        return 1;
    }
    this->shields.SetSize(4, -1);
    unk_6B0BD0.SetSize(0, -1);
    unk_6B0BD0.SetAtGrow(0, "");
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (this->shields.GetSize() <= index) {
            this->shields.SetSize(index + 1, -1);
        }
        this->shields[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Magic Items.txt", file)) {
        return 1;
    }
    this->magic_items.SetSize(10, -1);
    unk_6B1620.SetSize(0, -1);
    unk_6B1620.SetAtGrow(0, "");
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (this->magic_items.GetSize() <= index) {
            this->magic_items.SetSize(index + 1, -1);
        }
        this->magic_items[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Units.txt", file)) {
        return 1;
    }
    this->monsters.SetSize(64, -1);
    index = 64;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (line.Find("rem") == 0) {
            index++;
            continue;
        }
        if (line.Find("goto") == 0) {
            index = 26;
            continue;
        }
        if (this->monsters.GetSize() <= index) {
            this->monsters.SetSize(index + 1, -1);
        }
        this->monsters[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Humans.txt", file)) {
        return 1;
    }
    this->humans.SetSize(5, -1);
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (line.Find("rem") == 0) {
            index++;
            continue;
        }
        if (this->humans.GetSize() <= index) {
            this->humans.SetSize(index + 1, -1);
        }
        this->humans[index].ParseLine(line);
        index++;
    }
    file.Close();

    if (!OpenText(path + "Buildings.txt", file)) {
        return 1;
    }
    this->buildings.SetSize(20, -1);
    index = 1;
    while (file.ReadString(line)) {
        if (line.GetLength() < 2 || line[0] == '\t') {
            continue;
        }
        if (this->buildings.GetSize() <= index) {
            this->buildings.SetSize(index + 1, -1);
        }
        this->buildings[index].ParseLine(line);
        index++;
    }
    file.Close();

    return 0;
}

// 50D421
int GameDataRes::ParseDataBin(const CString& path) {
    CFile file;
    CString filename = path + "Data.bin";

    if (!file.Open(filename, CFile::shareDenyWrite, nullptr)) {
        try {
            Resources::StaticAddResFile("World_srv.res");
        } catch (CFileException* e) {
            e->Delete();
            return 1;
        }

        File2 file2;
        if (!file2.Open("World\\Data\\Data.bin", CFile::shareDenyWrite, nullptr)) {
            return 1;
        }

        CArchive ar(&file2, CArchive::load, 0x1000, nullptr);
        try {
            this->Serialize(ar);
        } catch (CArchiveException* e) {
            e->Delete();
            LogMessage("Invalid or outdated " + filename);
            ar.Close();
            return 2;
        }
        ar.Close();

        return 0;
    }

    CArchive ar(&file, CArchive::load, 0x1000, nullptr);
    try {
        this->Serialize(ar);
    } catch (CArchiveException* e) {
        e->Delete();
        LogMessage("Invalid or outdated " + filename);
        ar.Close();
        return 2;
    }
    ar.Close();

    return 0;
}

// 50D7C2
void GameDataRes::WriteDataBin(const CString& path) {
    CFile file;
    CString filename = path + "Data.bin";

    if (!file.Open(filename, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, nullptr)) {
        return;
    }

    CArchive ar(&file, CArchive::store);
    this->Serialize(ar);
    ar.Close();
    file.Close();
}

// 50a15f
int GameDataRes::ParseWorldIn(const CString& path) {
    if (this->loaded != 0) {
        return 0;
    }

    int result = this->ParseDataBin(path);
    if (result != 0) {
        if (result != 1) {
            return 2;
        }

        LogMessage("StaticData files not found");
        LogMessage("Parsing .txt files");

        int txt_result = this->ParseTxtFiles(path);
        if (txt_result != 0) {
            LogMessage("Error loading .txt files");
            return 1;
        }

        LogMessage("Writing new .bin file");
        this->WriteDataBin(path);
    }

    this->loaded = 1;
    return 0;
}
