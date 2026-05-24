#include "table.h"

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
