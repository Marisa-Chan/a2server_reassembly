#include "item.h"
#include "effect.h"
#include "game_app.h"
#include "table.h"

// sub_548860
bool Item::IsSimilar(Item* other)
{
    if (item_id != other->item_id) {
        return false;
    }

    int v1 = VMethod16();
    int v2 = other->VMethod16();
    if (v1 != 0 && v2 != 0) {
        return true;
    }
    if (v1 != 0 || v2 != 0) {
        return false;
    }

    POSITION pos1 = this->_effects.GetHeadPosition();
    POSITION pos2 = other->_effects.GetHeadPosition();

    while (pos1 != nullptr && pos2 != nullptr) {
        Effect* e1 = this->_effects.GetNext(pos1);
        Effect* e2 = other->_effects.GetNext(pos2);

        if (!e1->sub_53EDB0(e2)) {
            return false;
        }
    }

    return pos1 == nullptr && pos2 == nullptr;
}

// sub_54F634
void Armor::LoadInfo()
{
    EquipData* data = &this->world_equip->values[0];

    this->slot = 0;
    this->world_equip = &g_GameDataRes.armors[this->itemDataID];
    this->slot = (uint8_t)data->slot;

    if (this->slot >= 13) {
        CString s;
        s.Format("Invalid armor part %d created - discarded.", this->slot);
        LogMessage(s);
        return;
    }

    this->item_type = 1;
    MatShape* shape = &g_GameDataRes.shapes[this->shape_id];
    MatShape* mat = &g_GameDataRes.materials[this->material_id];

    this->protections.absorption = (int16_t)((double)data->absorption * mat->data.absorption * shape->data.absorption);
    this->protections.defense = (int16_t)((double)data->defense * mat->data.defense * shape->data.defense + 0.5);
    this->magic_volume = (int16_t)(mat->data.magic_volume * shape->data.magic_volume);
    this->weight = (int16_t)((double)data->weight * mat->data.weight * shape->data.weight + 0.5);

    this->VMethod15();

    this->item_id = (this->material_id << 12) | (this->slot << 8) | (this->shape_id << 5) | this->itemDataID;
}
