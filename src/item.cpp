#include "item.h"
#include "effect.h"
#include "game_app.h"
#include "table.h"
#include "packet.h"
#include "util.h"

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


