#include "table.h"

#include <cstdlib>


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
