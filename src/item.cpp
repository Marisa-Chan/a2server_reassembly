#include "item.h"
#include "effect.h"
#include "game_app.h"
#include "table.h"
#include "packet.h"
#include "util.h"
#include "file.h"

extern "C" void* dword_65FBB8; // Area for item names.


extern "C" Item* __cdecl sub_4F499B(uint8_t** packet_data)
{
    uint8_t* data = *packet_data;
    uint16_t encoded_item_id = *reinterpret_cast<uint16_t*>(data);
    data += sizeof(uint16_t);

    uint8_t flags = *data;
    data += sizeof(uint8_t);

    if (encoded_item_id == 0) {
        *packet_data = data;
        return new Item();
    }

    uint16_t count = 1;
    int32_t effect_bytes_remaining = 0;
    int32_t stored_price = 0;

    if ((flags & 0x80) != 0) {
        count = flags & 0x3F;
    } else if ((flags & 0x20) != 0) {
        effect_bytes_remaining = flags & 0x0F;
        stored_price = *reinterpret_cast<int32_t*>(data);
        data += sizeof(int32_t);
    } else {
        count = *reinterpret_cast<uint16_t*>(data);
        data += sizeof(uint16_t);
    }

    Item* item = nullptr;
    uint8_t shape_id = static_cast<uint8_t>((encoded_item_id >> 5) & 0x07);
    uint8_t material_id = static_cast<uint8_t>((encoded_item_id >> 12) & 0x0F);
    uint8_t item_data_id = static_cast<uint8_t>(encoded_item_id & 0x1F);
    uint8_t slot = static_cast<uint8_t>((encoded_item_id >> 8) & 0x0F);

    if (slot == 1) {
        item = new Weapon(shape_id, material_id, item_data_id);
    } else if (slot == 2) {
        item = new Shield(shape_id, material_id, item_data_id);
    } else if (slot == 0x0E) {
        item = new Item(g_GameDataRes.magic_items[item_data_id].name);
    } else {
        item = new Armor(shape_id, material_id, item_data_id);
    }

    if (effect_bytes_remaining == 0) {
        item->count = count;
    } else {
        while (effect_bytes_remaining != 0) {
            Effect* effect = new Effect();
            effect->effect_id = *data;
            data += sizeof(uint8_t);

            effect->damage_min = *data;
            effect->damage_spread = 0;
            data += sizeof(uint8_t);
            effect_bytes_remaining -= 1;

            if (effect->effect_id == 0x29) {
                data += sizeof(uint8_t);
                effect->spell_value = *data;
                data += sizeof(uint8_t);
                effect_bytes_remaining -= 1;
            } else if (effect->effect_id == 0x2C || effect->effect_id == 0x2D || effect->effect_id == 0x2E || effect->effect_id == 0x2F || effect->effect_id == 0x30) {
                data += sizeof(uint8_t);
                effect->damage_spread = *data;
                data += sizeof(uint8_t);
                effect_bytes_remaining -= 1;
            }

            item->_effects.AddTail(effect);
        }
    }

    item->VMethod15();

    if (stored_price == 2 && !item->_effects.IsEmpty()) {
        Effect* effect = new Effect();
        effect->effect_id = 1;
        effect->spell_or_damage = 2;
        effect->spell_value = 0;
        item->_effects.AddTail(effect);
        item->VMethod15();
    }

    item->sub_54A0BE();

    if (item->sub_548F07() == 0) {
        while (item->magic_volume < 0) {
            Effect* first_effect = item->_effects.IsEmpty() ? nullptr : item->_effects.GetHead();
            if (first_effect == nullptr || first_effect->full_magic_value < 2) {
                break;
            }

            first_effect->full_magic_value -= 1;
            item->sub_54A0BE();
        }
    } else if (item->material_id != 4 && item->material_id != 2) {
        Effect* first_effect = item->_effects.IsEmpty() ? nullptr : item->_effects.GetHead();
        if (first_effect != nullptr && first_effect->effect_id == 2 && first_effect->full_magic_value > 3) {
            first_effect->spell_or_damage = 3;
            first_effect->spell_value = 0;
        }
    }

    *packet_data = data;
    return item;
}

// 54840E
Item::~Item() {
}

// sub_548860
int Item::IsSimilar(Item* other)
{
    if (item_id != other->item_id) {
        return 0;
    }

    int v1 = VMethod16();
    int v2 = other->VMethod16();
    if (v1 != 0 && v2 != 0) {
        return 1;
    }
    if (v1 != 0 || v2 != 0) {
        return 0;
    }

    POSITION pos1 = this->_effects.GetHeadPosition();
    POSITION pos2 = other->_effects.GetHeadPosition();

    while (pos1 != nullptr && pos2 != nullptr) {
        Effect* e1 = this->_effects.GetNext(pos1);
        Effect* e2 = other->_effects.GetNext(pos2);

        if (!e1->sub_53EDB0(e2)) {
            return 0;
        }
    }

    return pos1 == nullptr && pos2 == nullptr;
}


void Item::StoreToPacket(PacketUnitStateVec* pkt, int arg)
{
    //549afb
    uint8_t *pdata = pkt->data + pkt->data_size;

    SetU16(pdata, item_id);

    //printf("StoreToPacket item_id %x item_type %d count %d\n", item_id, item_type, count);

    pdata[2] = 0;

    if (_effects.IsEmpty() || item_type == 3 || item_type == 4 || count == 0)
    {
        if (count < 0x40)
        {
            pdata[2] = count | 0x80;
            pkt->data_size += 3;
        }
        else
        {
            SetU16(pdata + 3, count);
            pkt->data_size += 5;
        }
    }
    else
    {
        pdata[2] |= 0x20;
        SetU32(pdata + 3, _exp);

        int32_t cnt_flg = 0;

        pkt->data_size += 7;

        for (POSITION pos = _effects.GetHeadPosition(); pos != nullptr;)
        {
            Effect* eff = _effects.GetNext(pos);
            if (eff->effect_id != 1)
            {
                pkt->data[pkt->data_size] = eff->effect_id;
                pkt->data_size++;

                pkt->data[pkt->data_size] = eff->spell_or_damage & 0xff;
                pkt->data_size++;

                if (eff->effect_id == 0x29)
                {
                    pdata[2] |= 0x10;

                    pkt->data[pkt->data_size] = 0x32;
                    pkt->data_size++;

                    pkt->data[pkt->data_size] = eff->spell_value & 0xff;
                    pkt->data_size++;

                    cnt_flg += 2;
                }
                else if (eff->effect_id == 0x2c ||
                        eff->effect_id == 0x2d ||
                        eff->effect_id == 0x2e ||
                        eff->effect_id == 0x2f || 
                        eff->effect_id == 0x30) 
                {
                        pkt->data[pkt->data_size] = eff->effect_id;
                        pkt->data_size++;

                        pkt->data[pkt->data_size] = (eff->spell_or_damage >> 8) & 0xff;
                        pkt->data_size++;
                        cnt_flg += 2;
                }
                else
                    cnt_flg++;
            }
        }

        pdata[2] = (pdata[2] & 0xf0) | cnt_flg;
    }

    if (TokenID != 0)
    {
        pdata[2] |= 0x40;
        TokenID = 0;
    }

    if (arg != 0)
    {
        pkt->data[pkt->data_size] = field11_0x4d;
        pkt->data_size++;
    }
}


// sub_54F634
void Armor::LoadInfo()
{
    this->slot = 0;
    this->world_equip = &g_GameDataRes.armors[this->itemDataID];

    EquipData* data = &this->world_equip->values[0];
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

// sub_550E26
void Weapon::LoadEquipInfo(WorldEquip* params)
{
    MatShape* shape = &g_GameDataRes.shapes[this->shape_id];
    MatShape* mat = &g_GameDataRes.materials[this->material_id];
    EquipData* data = &params->values[0];

    double damage_mult = (double)mat->data.damage * shape->data.damage;
    this->hit_values.hand_damage_min = (uint8_t)((double)data->damage_min * damage_mult + 0.5);
    this->hit_values.hand_damage_spread = (uint8_t)((double)data->damage_max * damage_mult - this->hit_values.hand_damage_min + 0.5);

    this->hit_values.attack = (int16_t)((double)data->attack * mat->data.attack * shape->data.attack + 0.5);
    this->protections.defense = (int16_t)((double)data->defense * mat->data.defense * shape->data.defense + 0.5);

    this->magic_volume = (int16_t)(mat->data.magic_volume * shape->data.magic_volume);
    this->weight = (int16_t)((double)data->weight * mat->data.weight * shape->data.weight + 0.5);

    if (data->range == -1) {
        this->range = 1;
    } else {
        this->range = (uint8_t)data->range;
    }
}



// 475988
void LoadItemNames() {
    File2 f;
    f.Open("world\\data\\itemname.bin", 0);
    uint32_t len = f.GetLength();
    int32_t count = len / sizeof(uint16_t);
    uint16_t* data = (uint16_t*)malloc(count * sizeof(uint16_t));
    f.Read(data, count * sizeof(uint16_t));
    f.Close();

    for (int32_t i = 0; i < count; i++) {
        unk_660DA8[data[i]] = txt_itemname.GetLine(i);
    }
    free(data);

    f.Open("world\\data\\itemname.pkt", 0);
    uint32_t pktSize = f.GetLength();
    dword_65FBB8 = malloc(pktSize);
    f.Read(dword_65FBB8, pktSize);
    f.Close();

    ItemNamePktEntry* entry = (ItemNamePktEntry*)((uint8_t*)dword_65FBB8 + 9);
    for (int32_t i = 0; i < count; i++) {
        uint16_t id = entry->id;
        if ((id & 0xF00) == 0xE00) {
            MagicItem* magic_item = &g_GameDataRes.magic_items[id & 0xFF];
            *(int32_t*)&entry->data[1] = magic_item->values[0].shape; // Maybe `data` is actually `int32_t`?
        }
        unk_660D28[id] = entry;
        entry = (ItemNamePktEntry*)((uint8_t*)entry + entry->data_len + 7);
    }
}