#include "item.h"

#include "constants.h"
#include "effect.h"
#include "game_app.h"
#include "inventory.h"
#include "spell.h"
#include "table.h"
#include "packet.h"
#include "util.h"
#include "file.h"
#include "server.h"
#include "unit.h"
#include "player.h"

void* g_ItemNamePktData; // Area for item names.


CMapWordToPtr g_ItemNamePktMap;
CMapWordToPtr g_ItemNameMap;


// Deserialize item.
// 4f499b (client: 4f63b9)
Item* __cdecl ReadItem(uint8_t** packet_data) {
    uint8_t* data = *packet_data;
    ItemId encoded_item_id = *reinterpret_cast<uint16_t*>(data);
    data += sizeof(uint16_t);

    ItemWireFlags flags = *data;
    data += sizeof(uint8_t);

    if (encoded_item_id == ItemId::NONE) {
        *packet_data = data;
        return new Item();
    }

    uint16_t count = 1;
    int32_t effect_bytes_remaining = 0;
    int32_t stored_price = 0;

    if (flags.is_compact_count) {
        count = flags.compact_count;
    } else if (flags.has_magic_effects) {
        effect_bytes_remaining = flags.effect_pair_count;
        stored_price = *reinterpret_cast<int32_t*>(data);
        data += sizeof(int32_t);
    } else {
        count = *reinterpret_cast<uint16_t*>(data);
        data += sizeof(uint16_t);
    }

    Item* item = nullptr;
    uint8_t shape_id = encoded_item_id.shape_id;
    uint8_t material_id = encoded_item_id.material_id;
    uint8_t item_data_id = encoded_item_id.equip_id;
    uint8_t slot = encoded_item_id.slot;

    if (slot == item_slot::weapon) {
        item = new Weapon(shape_id, material_id, item_data_id);
    } else if (slot == item_slot::shield) {
        item = new Shield(shape_id, material_id, item_data_id);
    } else if (slot == item_slot::magic_item) {
        item = new Item(g_GameDataRes.magic_items[encoded_item_id.magic_item_id].name);
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

            if (effect->effect_id == modifier::castspell) {
                data += sizeof(uint8_t);
                effect->spell_value = *data;
                data += sizeof(uint8_t);
                effect_bytes_remaining -= 1;
            } else if (effect->effect_id >= modifier::damagefire && effect->effect_id <= modifier::damageastral) {
                data += sizeof(uint8_t);
                effect->damage_spread = *data;
                data += sizeof(uint8_t);
                effect_bytes_remaining -= 1;
            }

            item->_effects.AddTail(effect);
        }
    }

    item->CalcValue();

    if (stored_price == 2 && !item->_effects.IsEmpty()) {
        Effect* effect = new Effect();
        effect->effect_id = modifier::price;
        effect->spell_or_damage = 2;
        effect->spell_value = 0;
        item->_effects.AddTail(effect);
        item->CalcValue();
    }

    item->RecalcFromTemplate();

    if (item->IsArtifact() == 0) {
        while (item->magic_volume < 0) {
            Effect* first_effect = item->_effects.IsEmpty() ? nullptr : item->_effects.GetHead();
            if (first_effect == nullptr || first_effect->full_magic_value < 2) {
                break;
            }

            first_effect->full_magic_value -= 1;
            item->RecalcFromTemplate();
        }
    } else if (item->material_id != material::gold && item->material_id != material::steel) {
        Effect* first_effect = item->_effects.IsEmpty() ? nullptr : item->_effects.GetHead();
        if (first_effect != nullptr && first_effect->effect_id == modifier::body && first_effect->full_magic_value > 3) {
            first_effect->spell_or_damage = 3;
            first_effect->spell_value = 0;
        }
    }

    *packet_data = data;
    return item;
}

IMPLEMENT_SERIAL(Item, Token, 1); // Runtime class definition at 6372b8.

// 547f80 (client: 52cb56) - default constructor (blank item)
Item::Item() : Token() {
    this->world_equip = nullptr;
    this->item_id = 0;
    this->count = 1;
    this->item_type = ItemType::NONE;
    this->shape_id = 0;
    this->material_id = 0;
    this->itemDataID = 0;
    this->weight = 1;
    this->TokenID = 0;
    this->shelf_refcount = 0;
    this->shelf_id = 0;
    this->special_flag = 0;
    this->upgraded_at_inn = 0;
}

// 54800e (client: 52cbe4) - construct item from type and subtype
Item::Item(uint8_t type, uint8_t subtype) : Token() {
    this->world_equip = nullptr;
    this->item_id = 0;
    this->count = 1;
    this->item_type = ItemType::NONE;
    this->shape_id = 0;
    this->material_id = 0;
    this->itemDataID = subtype;
    this->magic_volume = 0;
    this->weight = 1;
    this->TokenID = 0;
    this->shelf_refcount = 0;
    this->shelf_id = 0;
    this->special_flag = 0;
    this->upgraded_at_inn = 0;
    this->InitFromTemplate(type);
}

// 5480e3 (client: 52ccb9) - construct item from name string
Item::Item(const CString& name) : Token() {
    this->world_equip = nullptr;
    this->item_id = 0;
    this->count = 1;
    this->item_type = ItemType::NONE;
    this->shape_id = 0;
    this->material_id = 0;
    this->itemDataID = 0;
    this->magic_volume = 0;
    this->weight = 1;
    this->TokenID = 0;
    this->shelf_refcount = 0;
    this->shelf_id = 0;
    this->special_flag = 0;
    this->upgraded_at_inn = 0;
    int32_t count = g_GameDataRes.magic_items.GetSize();
    for (int32_t i = 1; i < count; i++) {
        if (g_GameDataRes.magic_items[i].name == name) {
            this->itemDataID = (int16_t)i;
            this->InitFromTemplate(item_slot::magic_item);
            return;
        }
    }
    LogMessage("Invalid Item " + name + " - no such ID");
}

// 5482ab (client: 52ce60) - init item fields from magic_items table
void Item::InitFromTemplate(uint8_t slot) {
    if (this->itemDataID >= g_GameDataRes.magic_items.GetSize()) {
        LogMessage("Error - creating invalid Item");
        return;
    }
    MagicItem& magic_item = g_GameDataRes.magic_items[this->itemDataID];
    this->world_equip = &magic_item;
    if (magic_item.name.Find("Potion") == 0) {
        this->item_type = ItemType::POTION;
    } else if (magic_item.name.Find("Book") == 0) {
        this->item_type = ItemType::BOOK;
    } else if (magic_item.name.Find("Scroll") == 0) {
        this->item_type = ItemType::SCROLL;
    } else if (magic_item.name.Find("SuperScroll") == 0) {
        this->item_type = ItemType::SCROLL;
    }
    this->item_id = ItemId::MakeMagic((uint8_t)this->itemDataID, slot, this->material_id);
    if (!magic_item.effect.IsEmpty()) {
        this->ApplyPropertyString(magic_item.effect);
    }
    this->CalcValue();
}

// 54842a (client: 52cfd3) - copy constructor
Item::Item(const Item* src) : Token(*src) {
    this->world_equip = src->world_equip;
    this->item_id = src->item_id;
    this->count = src->count;
    this->item_type = src->item_type;
    this->shape_id = src->shape_id;
    this->material_id = src->material_id;
    this->itemDataID = src->itemDataID;
    this->magic_volume = src->magic_volume;
    this->weight = src->weight;
    this->shelf_refcount = src->shelf_refcount;
    this->shelf_id = src->shelf_id;
    this->upgraded_at_inn = 0;
    POSITION pos = src->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = src->_effects.GetNext(pos);
        this->_effects.AddTail(new Effect(e));
    }
}

// 54840e (client: 52cfb7)
Item::~Item() {
}

// 548da5 (client: 52d541)
void Item::RegenTick() {
}

// 548d9a (client: 52d536)
void Item::ProcessEffects() {
}

// 57ce40 (client: 54e0d0)
int32_t Item::IsItem() {
    return 1;
}

// 548a59 (client: 52d335) - Equip.
Item* Item::Equip(Unit* pUnit) {
    if (pUnit->hp <= 0) {
        return this;
    }

    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        if (e->usage_type == Effect::USAGE_PERMANENT) {
            e->usage_type |= Effect::USAGE_SINGLE_USE;
        }
        e->RemoveFromUnit(pUnit);
    }

    pUnit->RecalcExperience(); // WAT: what's this for? Does nothing.

    int stat_bottle = this->world_equip->name.Find("ody") >= 0 ||
            this->world_equip->name.Find("ind") >= 0 ||
            this->world_equip->name.Find("eaction") >= 0 ||
            this->world_equip->name.Find("pirit") >= 0;

#ifdef A2CLIENT_PATCH
    // Retail dereferences pOwner unconditionally. The client-side roster preview
    // (CGameSession::LoadCharacterRosterEntry) equips save-file items on an ownerless
    // Human, so a stat potion in the equipment stream crashes the client on login.
    if (stat_bottle && pUnit->pOwner != nullptr && (GetTickCount() - pUnit->pOwner->last_save_tick > 15000)) {
#else
    if (stat_bottle && (GetTickCount() - pUnit->pOwner->last_save_tick > 15000)) {
#endif
        g_Server->SaveCharacter(static_cast<Humanoid*>(pUnit));
    }

    delete this;
    return nullptr;
}

// 548c04 (client: 52d476) - Unequip.
void Item::Unequip(Unit*) {
    LogMessage("Unknown item takeoff");
}

// 548dbf (client: 52d55b)
Item* Item::TakeOne() {
    this->count -= 1;
    Item* new_item = new Item(this);
    new_item->count = 1;
    return new_item;
}

// 57cdc0 (client: 54e050)
Item* Item::Clone() {
    return new Item(this);
}

// 548db0 (client: 52d54c)
int32_t Item::VMethod14(int, int) {
    return 0;
}

// 54872b (client: 52d131)
int32_t Item::CalcValue() {
    this->_exp = this->world_equip->Values()[0].shape;
    if (this->item_type == Item::BOOK) {
        Effect* first_effect = this->_effects.IsEmpty() ? nullptr : this->_effects.GetHead();
        if (first_effect == nullptr) {
            this->_exp = 0;
        } else {
            this->_exp = g_GameDataRes.spells[first_effect->spell_or_damage].Values()[0].book_cost;
        }
    }
    if (this->item_type == Item::SCROLL) {
        this->_exp = this->world_equip->Values()[0].shape;
    }
    return this->_exp;
}

// 548817 (client: 52d1e4)
int32_t Item::IsSimpleItem() {
    if (this->item_type == Item::POTION || this->item_type == Item::SCROLL || this->_effects.IsEmpty()) {
        return 1;
    }
    return 0;
}

// 549e86 (client: 52e1f8)
void Item::WriteState(PacketUnitStateVec* packet, uint8_t* slot) {
    if (this->_exp == -1) {
        slot[4] = ItemWireFlags::FLAG_USABLE;
        packet->AppendByteInt(modifier::price, 0, slot);
    } else {
        packet->AppendByteInt(modifier::price, this->_exp, slot);
        slot[4] |= ItemWireFlags::FLAG_ALL_CLASSES;
        if (this->item_type == Item::BOOK) {
            slot[4] &= ~ItemWireFlags::FLAG_WARRIOR;
        }
        this->WriteEffects(packet, slot);
        if (this->item_type == Item::POTION || this->item_type == Item::SCROLL) {
            slot[4] &= ~ItemWireFlags::FLAG_MAGIC_EFFECTS;
        }
    }
}

// 549f2c (client: 52e29e)
void Item::WriteEffects(PacketUnitStateVec* packet, uint8_t* slot) {
    if (this->_effects.IsEmpty()) {
        return;
    }
    slot[4] |= ItemWireFlags::FLAG_MAGIC_EFFECTS;
    packet->AppendByteByte(modifier::wire_effects_delimiter, 0, slot);
    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        packet->AppendByteByte(e->effect_id, e->damage_min, slot);
        if (e->effect_id == modifier::castspell) {
            slot[4] |= ItemWireFlags::FLAG_CAST_SPELL;
            packet->AppendByteByte(modifier::wire_castspell_power, (uint8_t)e->spell_value, slot);
        } else if (modifier::damagefire <= e->effect_id && e->effect_id <= modifier::damageastral) {
            packet->AppendByteByte(e->effect_id, e->damage_spread, slot);
        }
    }
}

// 55c2f1 (client: 53737c)
void Item::Serialize(CArchive& ar) {
    Token::Serialize(ar);
    if (ar.IsStoring()) {
        ar << (int32_t)this->_effects.GetCount();
        POSITION pos = this->_effects.GetHeadPosition();
        while (pos != nullptr) {
            Effect* e = this->_effects.GetNext(pos);
            ar.WriteObject(e);
        }
        ar << this->item_id;
        ar << this->count;
        ar << (uint8_t)this->item_type;
        ar << this->shape_id;
        ar << this->material_id;
        ar << this->magic_volume;
        ar << this->weight;
        ar << this->value1;
    } else {
        this->_effects.RemoveAll();
        int32_t cnt;
        ar >> cnt;
        for (int32_t i = 0; i < cnt; i++) {
            Effect* e = nullptr;
            ar >> e;
            this->_effects.AddTail(e);
        }
        ar >> this->item_id;
        ar >> this->count;
        ar >> (uint8_t&)this->item_type;
        ar >> this->shape_id;
        ar >> this->material_id;
        ar >> this->magic_volume;
        ar >> this->weight;
        ar >> this->value1;
        if (this->itemDataID < g_GameDataRes.magic_items.GetSize()) {
            this->world_equip = &g_GameDataRes.magic_items[this->itemDataID];
        } else {
            this->world_equip = nullptr;
        }
    }
}

// 548860 (client: 52d22d) - Is this item similar to another? Same non-magic item or same base with same magic.
int Item::IsSimilar(Item* other) {
    if (this->item_id != other->item_id) {
        return 0;
    }

    #ifdef A2SERVER_PATCH
    // bug_0011: 'this' can be a zombie item (zeroed vtable) - not similar
    if (*reinterpret_cast<void**>(this) == nullptr) {
        return 0;
    }
    #endif
    int v1 = this->IsSimpleItem();
    int v2 = other->IsSimpleItem();
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

        if (!e1->Matches(e2)) {
            return 0;
        }
    }

    return pos1 == nullptr && pos2 == nullptr;
}


// 549afb (client: 52dee1)
void Item::StoreToPacket(PacketUnitStateVec* pkt, int with_shelf_id) {
    uint8_t *pdata = pkt->data + pkt->data_size;

    SetU16(pdata, this->item_id);

    //printf("StoreToPacket item_id %x item_type %d count %d\n", item_id, item_type, count);

    ItemWireFlags wire_flags = 0;

    if (this->_effects.IsEmpty() || this->item_type == Item::POTION || this->item_type == Item::SCROLL || this->count == 0) {
        if (this->count < ItemWireFlags::COMPACT_COUNT_MAX) {
            wire_flags.is_compact_count = 1;
            wire_flags.compact_count = (uint8_t)this->count;
            pkt->data_size += 3;
        } else {
            SetU16(pdata + 3, this->count);
            pkt->data_size += 5;
        }
    } else {
        wire_flags.has_magic_effects = 1;
        SetU32(pdata + 3, this->_exp);

        int32_t cnt_flg = 0;

        pkt->data_size += 7;

        for (POSITION pos = this->_effects.GetHeadPosition(); pos != nullptr;) {
            Effect* eff = this->_effects.GetNext(pos);
            if (eff->effect_id != modifier::price) {
                pkt->data[pkt->data_size] = eff->effect_id;
                pkt->data_size++;

                pkt->data[pkt->data_size] = eff->spell_or_damage & 0xff;
                pkt->data_size++;

                if (eff->effect_id == modifier::castspell) {
                    wire_flags.has_cast_spell = 1;

                    pkt->data[pkt->data_size] = modifier::wire_castspell_power;
                    pkt->data_size++;

                    pkt->data[pkt->data_size] = eff->spell_value & 0xff;
                    pkt->data_size++;

                    cnt_flg += 2;
                } else if (eff->effect_id == modifier::damagefire ||
                        eff->effect_id == modifier::damagewater ||
                        eff->effect_id == modifier::damageair ||
                        eff->effect_id == modifier::damageearth ||
                        eff->effect_id == modifier::damageastral) {
                        pkt->data[pkt->data_size] = eff->effect_id;
                        pkt->data_size++;

                        pkt->data[pkt->data_size] = (eff->spell_or_damage >> 8) & 0xff;
                        pkt->data_size++;
                        cnt_flg += 2;
                } else {
                    cnt_flg++;
                }
            }
        }

        wire_flags.raw = (wire_flags.raw & 0xf0) | cnt_flg;
    }

    if (this->TokenID != 0) {
        wire_flags.is_newly_picked = 1;
        this->TokenID = 0;
    }

    pdata[2] = wire_flags;

    if (with_shelf_id != 0) {
        pkt->data[pkt->data_size] = this->shelf_id;
        pkt->data_size++;
    }
}

// 548c2e (client: 52d4a0)
void Item::ApplyEffects(Unit* unit) {
    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        e->ApplyToUnit(unit);
    }
}

// 548ce4 (client: 52d4eb)
void Item::RemoveEffects(Unit* unit) {
    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        e->ModifyVision(unit);
    }
}

// 548e4e (client: 52d5e7) - Find the cast-spell Effect on this item (returns nullptr if none)
Effect* Item::FindCastSpellEffect() {
    POSITION it = this->_effects.GetHeadPosition();
    while (it != nullptr) {
        Effect* e = this->_effects.GetNext(it);
        if (e->effect_id == modifier::castspell) {
            return e;
        }
    }
    return nullptr;
}

// 548f07 (client: none) - IsArtifact: returns 1 if the item has magic and price 2.
int Item::IsArtifact() {
    if (this->_effects.GetCount() != 0 && this->_exp == 2) {
        return 1;
    }
    return 0;
}

// 540daa (client: 52845f) - parse effect string into effects list
void ParseEffectString(const CString& effect_str, CList<Effect*>* effects) {
    CString name;
    CString str;

    str = effect_str;
    str.MakeLower();
    str.TrimLeft();
    str.TrimRight();
    str += ',';

    int32_t index = str.Find(',');
    while (index != -1) {
        name = str.Left(index);
        name.TrimLeft();
        name.TrimRight();

        Effect* effect = Effect::CreateFromString(name);
        if (effect != nullptr) {
            effects->AddTail(effect);
        }

        str = str.Right(str.GetLength() - index - 1);
        str.TrimLeft();
        str.TrimRight();
        index = str.Find(',');
    }
}

// 548f3f (client: 52d635) - Apply special property string to item effects, then call VMethod15
void Item::ApplyPropertyString(const CString& str) {
    ParseEffectString(str, &this->_effects);
    this->CalcValue();
}

// 548f6a (client: 52d660) - Returns 1 if this item template can be given via #create cheat
int Item::IsCheatCreatable() {
    if (this->world_equip == nullptr) {
        return 0;
    }
    return this->world_equip->shape_material_matrix[this->shape_id] & (1 << (this->material_id & 0x1f));
}

// 548faa (client: 52d6a0) - Add effect to item
void Item::AddEffect(Effect* effect) {
    int32_t usable_by = g_GameDataRes.magics[effect->effect_id].Values()[0].usable_by;
    if (usable_by != equip_suitable::both) {
        POSITION it = this->_effects.GetHeadPosition();
        while (it != nullptr) {
            Effect* e = this->_effects.GetNext(it);
            if ((usable_by & g_GameDataRes.magics[e->effect_id].Values()[0].usable_by) == 0) {
                // New effect is incompatible with an existing one.
                delete effect;
                return;
            }
        }
    }

    POSITION it = this->_effects.GetHeadPosition();
    while (it != nullptr) {
        Effect* e = this->_effects.GetNext(it);
        if (e->effect_id == effect->effect_id) {
            // Same effect - overwrite its value and discard the new one.
            e->full_magic_value = effect->full_magic_value;
            delete effect;
            return;
        }
    }

    this->_effects.AddTail(effect);
}

// 54f176 (client: 530f95) - total EffectPrice() of all effects
int32_t TotalEffectPrice(Item* item) {
    int32_t total = 0;
    POSITION it = item->_effects.GetHeadPosition();
    while (it != nullptr) {
        Effect* e = item->_effects.GetNext(it);
        total += e->EffectPrice();
    }
    return total;
}

// 54a0be (client: none) - Recalculate item properties from template data
void Item::RecalcFromTemplate() {
    if (this->IsSimpleItem() != 0) {
        return;
    }

    MatShape* shape = &g_GameDataRes.shapes[this->shape_id];
    MatShape* material = &g_GameDataRes.materials[this->material_id];
    this->magic_volume = (int16_t)(material->data.magic_volume * shape->data.magic_volume);

    this->magic_volume -= TotalEffectPrice(this);
}


IMPLEMENT_SERIAL(Armor, Item, 1); // Runtime class definition at 6372d0.

// 54f2e9 (client: 53109a) - default constructor
Armor::Armor() : Item() {
    this->material_id = 0;
    this->slot = 0;
    this->itemDataID = 0;
    this->special_flag = 0;
}

// 54f361 (client: 531112) - construct from name string
Armor::Armor(const CString& name) : Item() {
    this->slot = 0;
    this->itemDataID = 0;
    this->special_flag = 0;

    CString base_name;
    CString extra;
    g_GameDataRes.SplitItemName(&extra, &name, &base_name);

    this->shape_id = g_GameDataRes.FindShapeIndex(&base_name, &base_name);
    this->material_id = g_GameDataRes.FindMaterialIndex(&base_name, &base_name);
    g_GameDataRes.StripMaterialPrefix(this->material_id, &base_name);

    int32_t count = g_GameDataRes.armors.GetSize();
    int16_t item_data_id = 0;
    for (int32_t i = count - 1; i >= 1; i--) {
        if (base_name == g_GameDataRes.armors[i].name) {
            item_data_id = (int16_t)i;
            break;
        }
    }
    this->itemDataID = item_data_id;

    if (this->itemDataID == 0 || this->material_id > material::none) {
        LogMessage("Invalid armor '" + name + "' created - discarded.");
        return;
    }

    this->LoadInfo();
    if (extra.GetLength() > 0) {
        this->ApplyPropertyString(extra);
    }
}

// 54f5ab (client: 5312f4)
Armor::Armor(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id) : Item() {
    this->shape_id = shape_id;
    this->material_id = material_id;
    this->itemDataID = item_data_id;
    this->LoadInfo();
}

// 54f9ed (client: 5316b4) - copy constructor
Armor::Armor(const Armor* src) : Item(src) {
    this->slot = src->slot;
    this->protections = src->protections;
}

// 54f634 (client: 53137d)
void Armor::LoadInfo() {
    this->slot = 0;
    this->world_equip = &g_GameDataRes.armors[this->itemDataID];

    #ifdef A2SERVER_PATCH
    // bug_0012: equip values array can be empty (null m_pData) - skip the load
    if (this->world_equip->Values().GetData() == nullptr) {
        return;
    }
    #endif
    EquipData* data = &this->world_equip->Values()[0];
    this->slot = (uint8_t)data->slot;

    if (this->slot >= item_slot::count) {
        CString s;
        s.Format("Invalid armor part %d created - discarded.", this->slot);
        LogMessage(s);
        return;
    }

    this->item_type = ItemType::EQUIPMENT;
    MatShape* shape = &g_GameDataRes.shapes[this->shape_id];
    MatShape* mat = &g_GameDataRes.materials[this->material_id];

    this->protections.absorption = (int16_t)((double)data->absorption * mat->data.absorption * shape->data.absorption);
    this->protections.defense = (int16_t)((double)data->defense * mat->data.defense * shape->data.defense + 0.5);
    this->magic_volume = (int16_t)(mat->data.magic_volume * shape->data.magic_volume);
    this->weight = (int16_t)((double)data->weight * mat->data.weight * shape->data.weight + 0.5);

    this->CalcValue();

    this->item_id = ItemId::MakeEquip(this->material_id, this->slot, this->shape_id, (uint8_t)this->itemDataID);
}

// 550e26 (client: 5329b4)
void Weapon::LoadEquipInfo(WorldEquip* params) {
    MatShape* shape = &g_GameDataRes.shapes[this->shape_id];
    MatShape* mat = &g_GameDataRes.materials[this->material_id];

    #ifdef A2SERVER_PATCH
    // bug_0013: equip values array can be empty (null m_pData) - skip the load
    if (params->Values().GetData() == nullptr) {
        return;
    }
    #endif
    EquipData* data = &params->Values()[0];

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

// 54f9d1 (client: 531698)
Armor::~Armor() {
}

// 55cf2b (client: 5377f2)
void Armor::Serialize(CArchive& ar) {
    Item::Serialize(ar);
    this->protections.Serialize(ar);
    if (ar.IsStoring()) {
        ar.Write(&this->slot, 1);
    } else {
        ar.Read(&this->slot, 1);
        this->world_equip = &g_GameDataRes.armors[this->itemDataID];
    }
}

// 54fa6e (client: 531735)
Item* Armor::Equip(Unit* unit) {
    if (this->slot == item_slot::none) {
        LogMessage(CString("Illegal armor"));
        return this;
    }
    if (unit->IsHumanoid() == 0) {
        return this;
    }
    Humanoid* humanoid = static_cast<Humanoid*>(unit);
    Armor* old_armor = nullptr;
    if (humanoid->equipment[this->slot] != nullptr) {
        old_armor = static_cast<Armor*>(humanoid->equipment[this->slot]);
        old_armor->Unequip(unit);
    }
    humanoid->equipment[this->slot] = this;
    unit->equipment_extra.protections += this->protections;
    unit->protections += this->protections;
    unit->AddCarriedWeight(this->weight);
    unit->item_op_dirty_flags |= (UNIT_UPD_DEFENSE | UNIT_UPD_ABSORB);
    this->ApplyEffects(unit);
    return old_armor;
}

// 54fb7e (client: 531843)
void Armor::Unequip(Unit* unit) {
    if (unit->IsHumanoid() == 0) {
        return;
    }
    Humanoid* humanoid = static_cast<Humanoid*>(unit);
    unit->AddCarriedWeight(-this->weight);
    unit->equipment_extra.protections -= this->protections;
    unit->protections -= this->protections;
    humanoid->equipment[this->slot] = nullptr;
    unit->item_op_dirty_flags |= (UNIT_UPD_DEFENSE | UNIT_UPD_ABSORB);
    this->RemoveEffects(unit);
}

// 54fc19 (client: 5318dc)
Item* Armor::TakeOne() {
    this->count -= 1;
    Armor* copy = new Armor(this);
    copy->count = 1;
    return copy;
}

// 57d5b0 (client: 54e710)
Item* Armor::Clone() {
    return new Armor(this);
}

// 54f84d (client: 531584)
int32_t Armor::CalcValue() {
    double shape_price = g_GameDataRes.shapes[this->shape_id].data.price;
    double material_price = g_GameDataRes.materials[this->material_id].data.price;
    this->_exp = (int32_t)((double)this->world_equip->Values()[0].price * material_price * shape_price + 0.5);

    int32_t total = 0;
    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        if (e->effect_id == modifier::price) {
            this->_exp = e->full_magic_value;
            return this->_exp;
        }
        total += e->EffectPrice();
    }
    this->_exp += (int32_t)Effect::MagicPriceBonus(total);
    if (this->_exp > 19999999) {
        this->_exp = 19999999;
    }
    return this->_exp;
}

// 54fca8 (client: 531968)
void Armor::WriteState(PacketUnitStateVec* pkt, uint8_t* slot) {
    pkt->AppendByteInt(modifier::price, this->_exp, slot);
    slot[4] = 0;
    int32_t suitable_for = this->world_equip->Values()[0].suitable_for;
    if (suitable_for & equip_suitable::warrior) {
        slot[4] |= ItemWireFlags::FLAG_WARRIOR;
    }
    if (suitable_for & equip_suitable::mage) {
        slot[4] |= ItemWireFlags::FLAG_MAGE;
    }
    pkt->AppendByteByte(modifier::defence, this->protections.defense, slot);
    if (this->protections.absorption > 0) {
        pkt->AppendByteByte(modifier::absorbtion, this->protections.absorption, slot);
    }
    this->WriteEffects(pkt, slot);
}


IMPLEMENT_SERIAL(Shield, Item, 1); // Runtime class definition at 6372e8.

// 54fe1b (client: 531ad2) - default constructor
Shield::Shield() : Item() {
    this->itemDataID = 0;
    this->item_id = 0;
    this->special_flag = 0;
}

// 54fe8e (client: 531b45) - construct from name string
Shield::Shield(const CString& name) : Item() {
    this->itemDataID = 0;
    this->special_flag = 0;

    CString base_name;
    CString extra;
    g_GameDataRes.SplitItemName(&extra, &name, &base_name);

    this->shape_id = g_GameDataRes.FindShapeIndex(&base_name, &base_name);
    this->material_id = g_GameDataRes.FindMaterialIndex(&base_name, &base_name);
    g_GameDataRes.StripMaterialPrefix(this->material_id, &base_name);

    int32_t pos = base_name.Find(" Shield");
    if (pos > 0) {
        base_name = base_name.Left(pos);
    }

    int32_t count = g_GameDataRes.shields.GetSize();
    int16_t item_data_id = 0;
    for (int32_t i = count - 1; i >= 1; i--) {
        if (base_name == g_GameDataRes.shields[i].name) {
            item_data_id = (int16_t)i;
            break;
        }
    }
    this->itemDataID = item_data_id;

    if (this->itemDataID == 0 || this->material_id > material::none) {
        LogMessage("Invalid shield <" + name + "> created - discarded.");
        return;
    }

    this->LoadInfo();
    if (extra.GetLength() > 0) {
        this->ApplyPropertyString(extra);
    }
}

// 5500fa (client: 531da6)
Shield::Shield(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id) : Item() {
    this->shape_id = shape_id;
    this->material_id = material_id;
    this->itemDataID = item_data_id;
    this->special_flag = 0;
    this->LoadInfo();
}

// 550467 (client: 53208c) - copy constructor
Shield::Shield(const Shield* src) : Item(src) {
    this->protections = src->protections;
}

// 550183 (client: 531e2f)
void Shield::LoadInfo() {
    this->world_equip = &g_GameDataRes.shields[this->itemDataID];
    this->item_type = ItemType::EQUIPMENT;

    EquipData* data = &this->world_equip->Values()[0];
    MatShape* shape = &g_GameDataRes.shapes[this->shape_id];
    MatShape* material = &g_GameDataRes.materials[this->material_id];

    this->protections.absorption = (int16_t)((double)data->absorption * material->data.absorption * shape->data.absorption);
    this->protections.defense = (int16_t)((double)data->defense * material->data.defense * shape->data.defense + 0.5);
    this->magic_volume = (int16_t)(material->data.magic_volume * shape->data.magic_volume);
    this->weight = (int16_t)((double)data->weight * material->data.weight * shape->data.weight + 0.5);

    this->CalcValue();

    this->item_id = ItemId::MakeEquip(this->material_id, item_slot::shield, this->shape_id, (uint8_t)this->itemDataID);
}

// 55044b (client: 532070)
Shield::~Shield() {
}

// 55af5e (client: 536e3d)
void Shield::Serialize(CArchive& ar) {
    Item::Serialize(ar);
    this->protections.Serialize(ar);
    if (!ar.IsStoring()) {
        this->world_equip = &g_GameDataRes.shields[this->itemDataID];
    }
}

// 55056b (client: 53218d)
Item* Shield::Equip(Unit* unit) {
    if (this->itemDataID == 0) {
        LogMessage("Invalid shield");
        return this;
    }
    Shield* old_shield = nullptr;
    if (unit->shield != nullptr) {
        old_shield = unit->shield;
        old_shield->Unequip(unit);
    }
    if (unit->weapon != nullptr) {
        if (unit->weapon->world_equip->Values()[0].two_handed == 2) {
            Item* tmp = unit->Unequip(unit->weapon);
            unit->inventory->PutItemIntoBagAtDefault(tmp);
        }
    }
    unit->shield = this;
    unit->equipment_extra.protections += this->protections;
    unit->protections += this->protections;
    unit->AddCarriedWeight(this->weight);
    this->ApplyEffects(unit);
    unit->item_op_dirty_flags |= (UNIT_UPD_DEFENSE | UNIT_UPD_ABSORB);
    return old_shield;
}

// 550686 (client: 5322a3)
void Shield::Unequip(Unit* unit) {
    unit->AddCarriedWeight(-this->weight);
    unit->equipment_extra.protections -= this->protections;
    unit->protections -= this->protections;
    this->RemoveEffects(unit);
    unit->item_op_dirty_flags |= (UNIT_UPD_DEFENSE | UNIT_UPD_ABSORB);
    unit->shield = nullptr;
}

// 5504dc (client: 532101)
Item* Shield::TakeOne() {
    this->count -= 1;
    Shield* copy = new Shield(this);
    copy->count = 1;
    return copy;
}

// 57d660 (client: 54e7c0)
Item* Shield::Clone() {
    return new Shield(this);
}

// 5502c7 (client: 531f5c)
int32_t Shield::CalcValue() {
    double shape_price = g_GameDataRes.shapes[this->shape_id].data.price;
    double material_price = g_GameDataRes.materials[this->material_id].data.price;
    this->_exp = (int32_t)((double)this->world_equip->Values()[0].price * material_price * shape_price + 0.5);

    int32_t total = 0;
    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        if (e->effect_id == modifier::price) {
            this->_exp = e->full_magic_value;
            return this->_exp;
        }
        total += e->EffectPrice();
    }
    this->_exp += (int32_t)Effect::MagicPriceBonus(total);
    if (this->_exp > 19999999) {
        this->_exp = 19999999;
    }
    return this->_exp;
}

// 5506fc (client: 532317)
void Shield::WriteState(PacketUnitStateVec* pkt, uint8_t* slot) {
    pkt->AppendByteInt(modifier::price, this->_exp, slot);
    slot[4] = 0;
    int32_t suitable_for = this->world_equip->Values()[0].suitable_for;
    if (suitable_for & equip_suitable::warrior) {
        slot[4] |= ItemWireFlags::FLAG_WARRIOR;
    }
    if (suitable_for & equip_suitable::mage) {
        slot[4] |= ItemWireFlags::FLAG_MAGE;
    }
    pkt->AppendByteByte(modifier::defence, this->protections.defense, slot);
    if (this->protections.absorption > 0) {
        pkt->AppendByteByte(modifier::absorbtion, this->protections.absorption, slot);
    }
    this->WriteEffects(pkt, slot);
}


IMPLEMENT_SERIAL(Weapon, Item, 1); // Runtime class definition at 637300.

// 550872 (client: 532484) - default constructor
Weapon::Weapon() {
    this->range = 1;
    this->imbued_spell = nullptr;
    this->special_flag = 0;

    this->item_id = ItemId::MakeEquip(this->material_id, item_slot::weapon, this->shape_id, (uint8_t)this->itemDataID);
}

// 550929 (client: 53252a) - construct weapon from name string
Weapon::Weapon(const CString& name) : Item() {
    this->range = 1;
    this->itemDataID = 0;
    this->imbued_spell = nullptr;
    this->special_flag = 0;

    CString base_name;
    CString extra;
    g_GameDataRes.SplitItemName(&extra, &name, &base_name);

    this->shape_id = g_GameDataRes.FindShapeIndex(&base_name, &base_name);
    this->material_id = g_GameDataRes.FindMaterialIndex(&base_name, &base_name);

    int32_t count = g_GameDataRes.weapons.GetSize();
    int16_t found = 0;
    for (int32_t i = count - 1; i >= 1; i--) {
        if (base_name == g_GameDataRes.weapons[i].name) {
            found = (int16_t)i;
            break;
        }
    }
    this->itemDataID = found;

    if (this->itemDataID == 0) {
        LogMessage("Invalid weapon " + name + " - no such ID");
        return;
    }

    this->LoadInfo();
    if (extra.GetLength() > 0) {
        this->ApplyPropertyString(extra);
    }
}

// 550b8f (client: 532729)
Weapon::Weapon(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id) : Item() {
    this->shape_id = shape_id;
    this->material_id = material_id;
    this->itemDataID = item_data_id;
    this->special_flag = 0;
    this->LoadInfo();
}

// 550c23 (client: 5327bd)
void Weapon::LoadInfo() {
    this->range = 1;
    this->imbued_spell = nullptr;

    int32_t count = g_GameDataRes.weapons.GetSize();
    if (this->material_id > material::none || this->itemDataID >= count) {
        CString s;
        s.Format("Invalid weapon %d %d - no such ID", (int)this->material_id, (int)this->itemDataID);
        LogMessage(s);
        return;
    }

    this->world_equip = &g_GameDataRes.weapons[this->itemDataID];
    this->item_type = ItemType::WEAPON;
    this->LoadEquipInfo(this->world_equip);
    this->CalcValue();

    this->item_id = ItemId::MakeEquip(this->material_id, item_slot::weapon, this->shape_id, (uint8_t)this->itemDataID);
}

// 5511ee (client: 532cfa) - copy constructor
Weapon::Weapon(const Weapon* src) : Item(src) {
    this->range = src->range;
    this->hit_values = src->hit_values;
    this->protections = src->protections;
    if (src->imbued_spell != nullptr) {
        this->imbued_spell = new Spell(src->imbued_spell->spell_id);
    } else {
        this->imbued_spell = nullptr;
    }
}

// 551159 (client: 532c65)
Weapon::~Weapon() {
    if (this->imbued_spell != nullptr) {
        delete this->imbued_spell;
        this->imbued_spell = nullptr;
    }
}

// 55929f (client: 536298)
void Weapon::Serialize(CArchive& ar) {
    Item::Serialize(ar);
    this->hit_values.Serialize(ar);
    this->protections.Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->range;
        ar.WriteObject(this->imbued_spell);
    } else {
        ar >> this->range;
        this->imbued_spell = (Spell*)ar.ReadObject(RUNTIME_CLASS(Spell));
        this->world_equip = &g_GameDataRes.weapons[this->itemDataID];
    }
}

// 551b12 (client: 5335f5)
void Weapon::SetImbuedSpell(uint8_t spell_id) {
    if (this->imbued_spell != nullptr) {
        delete this->imbued_spell;
    }
    if (spell_id > spell::max_spell_id) {
        spell_id = spell::drain_life;
    }
    this->imbued_spell = new Spell(spell_id);
}

// 551bdb (client: 5336a8)
void Weapon::InitImbuedSpell() {
    Effect* effect = this->FindCastSpellEffect();
    if (effect != nullptr) {
        this->SetImbuedSpell(effect->damage_min);
    }
}

// 5512e7 (client: 532dee)
Item* Weapon::Equip(Unit* unit) {
    this->InitImbuedSpell();

    Item* old_weapon = nullptr;
    if (unit->weapon != nullptr) {
        old_weapon = unit->weapon;
        unit->weapon->Unequip(unit);
    }

    if (this->world_equip->Values()[0].two_handed == 2 && unit->shield != nullptr) {
        Item* tmp = unit->Unequip(unit->shield);
        unit->inventory->PutItemIntoBagAtDefault(tmp);
    }

    unit->weapon = this;
    int32_t attack_type = this->world_equip->Values()[0].attack_type;
    if (attack_type < weapon_attack::staff_threshold) {
        unit->equipment_extra.hit_values.hand_damage_min += this->hit_values.hand_damage_min;
        unit->equipment_extra.hit_values.hand_damage_spread += this->hit_values.hand_damage_spread;
        unit->equipment_extra.protections.defense += this->protections.defense;
        unit->equipment_extra.hit_values.some_damage2_min = this->hit_values.some_damage2_min;
        unit->equipment_extra.hit_values.some_damage2_spread = this->hit_values.some_damage2_spread;
        unit->equipment_extra.hit_values.spell_id = this->hit_values.spell_id;
        unit->equipment_extra.hit_values.attack += this->hit_values.attack;
        unit->hit_values.physical_damage_type = (uint8_t)attack_type;
    } else {
        if (attack_type == weapon_attack::staff_fire) {
            unit->equipment_extra.hit_values.some_damage2_min += this->hit_values.hand_damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread += this->hit_values.hand_damage_spread;
            unit->equipment_extra.hit_values.spell_id = spell::fire_arrow;
        } else if (attack_type == weapon_attack::staff_water) {
            unit->equipment_extra.hit_values.some_damage2_min += this->hit_values.hand_damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread += this->hit_values.hand_damage_spread;
            unit->equipment_extra.hit_values.spell_id = spell::fire_ball;
        }
        unit->equipment_extra.hit_values.attack = unit->hit_values.skill_levels[weapon_attack::unarmed];
        unit->hit_values.physical_damage_type = weapon_attack::unarmed;
    }
    unit->RecalcStats();
    if (unit->IsHumanoid() != 0) {
        if (this->world_equip->Values()[0].charge != -1) {
            unit->charge = (uint8_t)this->world_equip->Values()[0].charge;
        }
        if (this->world_equip->Values()[0].relax != -1) {
            unit->relax = (uint8_t)this->world_equip->Values()[0].relax;
        }
    }
    unit->max_range += (uint8_t)(this->range - 1);
    unit->AddCarriedWeight(this->weight);
    unit->item_op_dirty_flags |= (UNIT_UPD_DEFENSE | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
    this->ApplyEffects(unit);
    return old_weapon;
}

// 5515b9 (client: 5330b4)
void Weapon::Unequip(Unit* unit) {
    this->RemoveEffects(unit);
    int32_t attack_type = this->world_equip->Values()[0].attack_type;
    if (attack_type < weapon_attack::staff_threshold) {
        unit->equipment_extra.hit_values.hand_damage_min -= this->hit_values.hand_damage_min;
        unit->equipment_extra.hit_values.hand_damage_spread -= this->hit_values.hand_damage_spread;
        unit->equipment_extra.protections.defense -= this->protections.defense;
        unit->equipment_extra.hit_values.spell_id = 0;
        unit->equipment_extra.hit_values.some_damage2_min = 0;
        unit->equipment_extra.hit_values.some_damage2_spread = 0;
    } else if (attack_type == weapon_attack::staff_fire || attack_type == weapon_attack::staff_water) {
        unit->equipment_extra.hit_values.some_damage2_min -= this->hit_values.hand_damage_min;
        unit->equipment_extra.hit_values.some_damage2_spread -= this->hit_values.hand_damage_spread;
        unit->equipment_extra.hit_values.spell_id = 0;
    }
    unit->equipment_extra.hit_values.attack -= this->hit_values.attack;
    unit->hit_values.physical_damage_type = weapon_attack::unarmed;
    unit->RecalcStats();
    unit->max_range -= (uint8_t)(this->range - 1);
    unit->charge = 8;
    unit->relax = 4;
    unit->AddCarriedWeight(-this->weight);
    unit->item_op_dirty_flags |= (UNIT_UPD_DEFENSE | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
    if (this->imbued_spell != nullptr) {
        delete this->imbued_spell;
        this->imbued_spell = nullptr;
    }
    unit->weapon = nullptr;
}

// 5517c3 (client: 5332bc)
Item* Weapon::TakeOne() {
    this->count -= 1;
    Weapon* copy = new Weapon(this);
    copy->count = 1;
    return copy;
}

// 57d710 (client: 54e870)
Item* Weapon::Clone() {
    return new Weapon(this);
}

// 550fa6 (client: 532b26)
int32_t Weapon::CalcValue() {
    double shape_price = g_GameDataRes.shapes[this->shape_id].data.price;
    double material_price = g_GameDataRes.materials[this->material_id].data.price;
    this->_exp = (int32_t)((double)this->world_equip->Values()[0].price * material_price * shape_price + 0.5);

    int32_t total = 0;
    POSITION pos = this->_effects.GetHeadPosition();
    while (pos != nullptr) {
        Effect* e = this->_effects.GetNext(pos);
        if (e->effect_id == modifier::price) {
            this->_exp = e->full_magic_value;
            return this->_exp;
        } else if (e->effect_id == modifier::castspell) {
            this->_exp += e->CalcScrollPrice();
        } else {
            total += e->EffectPrice();
        }
    }
    this->_exp += (int32_t)Effect::MagicPriceBonus(total);
    if (this->_exp > 19999999) {
        this->_exp = 19999999;
    }
    return this->_exp;
}

// 551855 (client: 53334b)
void Weapon::WriteState(PacketUnitStateVec* pkt, uint8_t* slot) {
    pkt->AppendByteInt(modifier::price, this->_exp, slot);
    slot[4] = 0;
    int32_t suitable_for = this->world_equip->Values()[0].suitable_for;
    if (suitable_for & equip_suitable::warrior) {
        slot[4] |= ItemWireFlags::FLAG_WARRIOR;
        pkt->AppendByteByte(modifier::damagemin, this->hit_values.hand_damage_min, slot);
        pkt->AppendByteByte(modifier::damagemax, this->hit_values.hand_damage_spread, slot);
        if (this->hit_values.attack > 0) {
            pkt->AppendByteByte(modifier::tohit, this->hit_values.attack, slot);
        }
        if (this->protections.defense > 0) {
            pkt->AppendByteByte(modifier::defence, this->protections.defense, slot);
        }
        if (this->range > 1) {
            pkt->AppendByteByte(modifier::itemlore, this->range, slot); // Don't know why it uses `itemlore`. Maybe it's a reused property?
        }
    }
    if (suitable_for & equip_suitable::mage) {
        slot[4] |= ItemWireFlags::FLAG_MAGE;
    }
    if (this->hit_values.some_damage2_spread != 0) {
        uint8_t damage_sphere = 0;
        switch (this->hit_values.spell_id) {
        case magic_sphere::fire:   damage_sphere = modifier::damagefire; break;
        case magic_sphere::water:  damage_sphere = modifier::damagewater; break;
        case magic_sphere::air:    damage_sphere = modifier::damageair; break;
        case magic_sphere::earth:  damage_sphere = modifier::damageearth; break;
        case magic_sphere::astral: damage_sphere = modifier::damageastral; break;
        }
        pkt->AppendByteByte(damage_sphere, this->hit_values.some_damage2_min, slot);
        pkt->AppendByteByte(modifier::damagemax, this->hit_values.some_damage2_spread, slot);
    }
    this->WriteEffects(pkt, slot);
    slot[4] &= ~ItemWireFlags::FLAG_CAST_SPELL;
}


// 475988 (client: 4796d8)
void LoadItemNames() {
    File2 f;
    f.Open("world\\data\\itemname.bin", 0);
    uint32_t len = f.GetLength();
    int32_t count = len / sizeof(uint16_t);
    uint16_t* data = (uint16_t*)malloc(count * sizeof(uint16_t));
    f.Read(data, count * sizeof(uint16_t));
    f.Close();

    for (int32_t i = 0; i < count; i++) {
        g_ItemNameMap[data[i]] = txt_itemname.GetLine(i);
    }
    free(data);

    f.Open("world\\data\\itemname.pkt", 0);
    uint32_t pktSize = f.GetLength();
    g_ItemNamePktData = malloc(pktSize);
    f.Read(g_ItemNamePktData, pktSize);
    f.Close();

    ItemNamePktEntry* entry = (ItemNamePktEntry*)((uint8_t*)g_ItemNamePktData + 9);
    for (int32_t i = 0; i < count; i++) {
        ItemId id = entry->id;
        if (id.slot == item_slot::magic_item) {
            MagicItem* magic_item = &g_GameDataRes.magic_items[id.magic_item_id];
            *(int32_t*)&entry->data[1] = magic_item->Values()[0].shape; // Maybe `data` is actually `int32_t`?
        }
        g_ItemNamePktMap[id] = entry;
        entry = (ItemNamePktEntry*)((uint8_t*)entry + entry->data_len + 7);
    }
}
