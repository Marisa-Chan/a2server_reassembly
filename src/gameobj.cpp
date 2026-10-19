#include "constants.h"
#include "gameobj.h"
#include "main_window.h"
#include "item.h"
#include "game_app.h"
#include "spell.h"
#include "unit.h"
#include "packet.h"
#include "gfx.h"
#include "ingame.h"
#include "map_stuff.h"
#include "quest.h"
#include "quest_map.h"
#include "sound.h"
#include "mouse.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

CStringArray g_CUnitMaterialSpritePaths; //660e70

char g_Destination[0x400]; //659558 TokenEntry::FUN_00439973 hint buffer
char byte_65F630[1024]; //65f630 CUnit::FUN_0046d0f7 spell-list hint buffer

//GetRuntimeClass 461020
IMPLEMENT_DYNAMIC(CGameObject, CObject);

//GetRuntimeClass 461070
IMPLEMENT_DYNAMIC(CBackPack, CGameObject);

//GetRuntimeClass 4610a0
IMPLEMENT_DYNAMIC(CProjectile, CGameObject);

//GetRuntimeClass 461080
IMPLEMENT_DYNAMIC(CUnit, CGameObject);

//GetRuntimeClass 461090
IMPLEMENT_DYNAMIC(CAirUnit, CUnit);

//GetRuntimeClass 461030
IMPLEMENT_DYNAMIC(CStructure, CGameObject);

//GetRuntimeClass 461040
IMPLEMENT_DYNAMIC(CBridge, CStructure);

//GetRuntimeClass 461050
IMPLEMENT_DYNAMIC(CVerticalWoodenBridge, CBridge);

//GetRuntimeClass 461060
IMPLEMENT_DYNAMIC(CHorisontalWoodenBridge, CBridge);

CGameObject::CGameObject() {
    //4610b0
    this->unit_id = 0;
    this->x_pos = 0;
    this->y_pos = 0;
    this->x_pos2 = 0;
    this->y_pos2 = 0;
    this->z_pos = 0;
    this->dir = 0;
    this->phase = 0;
    this->speed = 0;
    this->hp = 0;
    this->hp_max = 0;
    this->field_0x84 = -1;
    this->controlGroupMask = 0;
    this->m_bSelected = 0;
    this->bIsBlocked = 0;
    this->action_segments = 0;
    this->last_action = 0;
    this->action = 0;
    this->scan_range = 0;
    this->action_x = 0;
    this->action_y = 0;
    this->action_z = 0;
    this->action_dir = 0;
    this->action_phase = 0;
    this->move_accum_x = 0;
    this->move_accum_y = 0;
    this->action_spell = 0;
    this->availableSpellMask = 0;
    this->spells = 0;
    this->activeSpellEffectMask = 0;
    this->m_bSelectionDirty = 1;
    this->active_spell = 0;

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    if (wnd) {
        this->pMapObject = wnd->vis_map_context;
    }

    this->shopInventoryVisibleStart = 0;
    this->str1[0] = 0;
    this->str2[0] = 0;
}

CGameObject::CGameObject(const CGameObject* obj) {
    //4612f2
    this->unit_id = obj->unit_id;
    this->x_pos = obj->x_pos;
    this->y_pos = obj->y_pos;
    this->z_pos = obj->z_pos;
    this->map_player = obj->map_player;
    this->typeId = obj->typeId;
    this->face = obj->face;
    this->x_pos2 = obj->x_pos2;
    this->y_pos2 = obj->y_pos2;
    this->field_0x34 = obj->field_0x34;
    this->tileX = obj->tileX;
    this->tileY = obj->tileY;
    this->mapBoundsLeft = obj->mapBoundsLeft;
    this->mapBoundsTop = obj->mapBoundsTop;
    this->mapBoundsRight = obj->mapBoundsRight;
    this->mapBoundsBottom = obj->mapBoundsBottom;
    this->mapLayerActive = obj->mapLayerActive;
    this->centerScreenX = obj->centerScreenX;
    this->centerScreenY = obj->centerScreenY;
    this->screenX = obj->screenX;
    this->screenY = obj->screenY;
    this->terrainHeightOffset = obj->terrainHeightOffset;
    this->dir = obj->dir;
    this->phase = obj->phase;
    this->last_action = obj->last_action;
    this->bIsBlocked = obj->bIsBlocked;
    this->m_bSelected = obj->m_bSelected;
    this->field_0x84 = obj->field_0x84;
    this->controlGroupMask = obj->controlGroupMask;
    this->action = obj->action;
    this->action_x = obj->action_x;
    this->action_y = obj->action_y;
    this->action_z = obj->action_z;
    this->action_dir = obj->action_dir;
    this->action_phase = obj->action_phase;
    this->action_target = obj->action_target;
    this->move_accum_x = obj->move_accum_x;
    this->move_accum_y = obj->move_accum_y;
    this->action_spell = obj->action_spell;
    this->availableSpellMask = obj->availableSpellMask;
    this->action_segments = obj->action_segments;
    this->turn_angle = obj->turn_angle;
    this->shopInventoryVisibleStart = obj->shopInventoryVisibleStart;
    this->pMapObject = obj->pMapObject;
    this->hp = obj->hp;
    this->field_0x106 = obj->field_0x106;
    this->hp_max = obj->hp_max;
    this->scan_range = obj->scan_range;
    this->speed = obj->speed;
    this->carrying_weight_100g = obj->carrying_weight_100g;
    this->m_bSelectionDirty = obj->m_bSelectionDirty;

    this->effect_timers.Copy(obj->effect_timers);
    this->transientVisualElements.Copy(obj->transientVisualElements);
    this->actionTargets.Copy(obj->actionTargets);

    for (int i = 0; i < obj->tokenEntries.GetSize(); i++) {
        this->tokenEntries.Add(new TokenEntry(obj->tokenEntries[i]));
    }

    strcpy(this->str1, obj->str1);
    strcpy(this->str2, obj->str2);
}


void CGameObject::AssertValid() const { //461dfa
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    if ((mwnd->dialogsMask & 1) != 0 && 
        (this->x_pos < 1792 || this->y_pos < 1792 || 
        (this->pMapObject->map_width - 7) * 256 <= this->x_pos ||
        (this->pMapObject->map_height - 7) * 256 <= this->y_pos || 
         this->typeId < 0 || this->typeId > 256 ||
         this->action_x < -256 || this->action_y < -256 ||
         (this->pMapObject->map_width - 8) * 256 <= this->action_x || 
         (this->pMapObject->map_height - 8) * 256 <= this->action_y)) {
        CString str;
        str.Format("Shit! Invalid GameObject #%d coordinates X:%d Y:%d player:%d picture:%d", this->unit_id, this->x_pos >> 8, this->y_pos >> 8, this->map_player->index, this->typeId);

        AfxMessageBox(str);
    }
}

void CGameObject::Dump(CDumpContext& dc) const { //461de0
    //dc << "CGameObject";
}

CGameObject::~CGameObject() { //46178a
    for (int i = 0; i < this->tokenEntries.GetSize(); i++) {
        delete this->tokenEntries[i];
    }

    this->tokenEntries.RemoveAll();
}


void CGameObject::FUN_0041f180(int32_t grp) { //41f180
    this->controlGroupMask |= 1 << grp;
    this->m_bSelectionDirty = 1;
}


void CGameObject::FUN_0046190d() { //46190d
    BigStruct2* map = this->pMapObject;

    this->tileX = this->x_pos2 >> 8;
    this->tileY = this->y_pos2 >> 8;

    this->mapBoundsLeft = ((this->x_pos2 - 0x80) >> 8) - map->view_x;
    this->mapBoundsTop = ((this->y_pos2 - 0x80) >> 8) - map->view_y;
    this->mapBoundsRight = ((this->x_pos2 + 127 + (this->VMethod4() - 1) * 0x100) >> 8) - map->view_x;
    this->mapBoundsBottom = ((this->y_pos2 + 127 + (this->VMethod5() - 1) * 0x100) >> 8) - map->view_y;

    this->mapLayerActive = 1;
    if (this->mapBoundsLeft < -3) {
        this->mapBoundsLeft = -3;
    }
    if (this->mapBoundsLeft >= map->view_tiles_x + 3) {
        this->mapLayerActive = 0;
    }
    if (this->mapBoundsRight < -3) {
        this->mapLayerActive = 0;
    }
    if (this->mapBoundsRight >= map->view_tiles_x + 3) {
        this->mapBoundsRight = map->view_tiles_x + 2;
    }
    if (this->mapBoundsTop < -3) {
        this->mapBoundsTop = -3;
    }
    if (this->mapBoundsTop >= map->view_tiles_y + 7) {
        this->mapLayerActive = 0;
    }
    if (this->mapBoundsBottom < -3) {
        this->mapLayerActive = 0;
    }
    if (this->mapBoundsBottom >= map->view_tiles_y + 7) {
        this->mapBoundsBottom = map->view_tiles_y + 6;
    }

    this->screenX = (this->x_pos2 >> 3) - map->view_x * 32;
    this->screenY = (this->y_pos2 >> 3) - map->view_y * 32;

    this->centerWorldX8 = this->x_pos2 - 128 + this->VMethod4() * 128;
    this->centerWorldY8 = this->y_pos2 - 128 + this->VMethod5() * 128;

    this->centerScreenX = (this->centerWorldX8 >> 3) - map->view_x * 32;
    this->centerScreenY = (this->centerWorldY8 >> 3) - map->view_y * 32;

    int32_t fracX = (this->centerWorldX8 >> 3) & 0x1f;
    int32_t fracY = (this->centerWorldY8 >> 3) & 0x1f;
    int32_t idx = (this->centerWorldX8 >> 8) + (this->centerWorldY8 >> 8) * map->map_width;
    const uint8_t* heights = map->field_0x80->GetMapHeights();

    int32_t h00 = (int8_t)heights[idx];
    int32_t h10 = (int8_t)heights[idx + 1];
    int32_t h01 = (int8_t)heights[idx + map->map_width];
    int32_t h11 = (int8_t)heights[idx + 1 + map->map_width];

    int32_t top = h00 + (h10 - h00) * fracX / 32;
    int32_t bottom = h01 + (h11 - h01) * fracX / 32;
    this->terrainHeightOffset = top + (bottom - top) * fracY / 32;
}


void CGameObject::FUN_00461c41(int32_t arg1, CGameObject* arg2) { //461c41
    if (this->mapLayerActive == 0) {
        return;
    }

    BigStruct2* map = this->pMapObject;
    CGameObject** layer;
    switch (arg1) {
    case 0:
        layer = map->tile_backpacks;
        break;
    case 1:
        layer = map->tile_structures;
        for (int32_t x = this->mapBoundsLeft; x <= this->mapBoundsRight; x++) {
            for (int32_t y = this->mapBoundsTop; y <= this->mapBoundsBottom; y++) {
                layer[x + 3 + (y + 3) * (map->view_tiles_x + 6)] = arg2;
            }
        }
        return;
    case 2:
        layer = map->tile_units;
        break;
    case 3:
        layer = map->tile_air_units;
        break;
    case 4:
        layer = map->tile_special_units;
        break;
    default:
        return;
    }

    int32_t x = ((this->x_pos2 - 0x80 + this->VMethod4() * 0x80) >> 8) - map->view_x;
    int32_t y = ((this->y_pos2 - 0x80 + this->VMethod5() * 0x80) >> 8) - map->view_y;
    if (x >= -4 && y >= -4 && x < map->view_tiles_x + 3 && y < map->view_tiles_y + 7) {
        layer[x + 3 + (y + 3) * (map->view_tiles_x + 6)] = arg2;
    }
}


int32_t CGameObject::FUN_00461f8e(int32_t x, int32_t y) { //461f8e
    int32_t dx = x - this->x_pos;
    int32_t dy = y - this->y_pos;
    int32_t adx = abs(dx);
    int32_t ady = abs(dy);

    int32_t dir;
    if (adx >= ady * 4) {
        dir = 0;
    } else if (adx * 3 >= ady * 4) {
        dir = 1;
    } else if (ady >= adx * 4) {
        dir = 4;
    } else if (ady * 3 >= adx * 4) {
        dir = 3;
    } else {
        dir = 2;
    }

    if (dy > 0) {
        if (dx > 0) {
            dir += 4;
        } else {
            dir = 12 - dir;
        }
    } else {
        if (dx < 0) {
            dir += 12;
        } else {
            dir = 4 - dir;
        }
    }
    return dir & 0xf;
}


int32_t CGameObject::FUN_00462405(int32_t effect_id) { //462405
    for (int32_t i = 0; i < this->effect_timers.GetSize(); i++) {
        if ((this->effect_timers[i] >> 16) == effect_id) {
            return i;
        }
    }
    return -1;
}


int32_t CGameObject::GetControlGroup() { //41f220
    for (int32_t grp = 0; grp < 0x20; grp++) {
        if (this->controlGroupMask & (1 << grp)) {
            return grp;
        }
    }
    return -1;
}


void CGameObject::FUN_0041f1f0(int32_t grp) { //41f1f0
    this->controlGroupMask &= ~(1 << grp);
}


void CGameObject::FUN_0041f150(int32_t grp) { //41f150
    this->controlGroupMask = 1 << grp;
    this->m_bSelectionDirty = 1;
}


int32_t CGameObject::FUN_0041f130() { //41f130
    return this->bIsBlocked;
}



void CGameObject::VMethod1(int32_t arg1) { //46f430
    this->m_bSelected = arg1;
    this->m_bSelectionDirty = 1;
}


int32_t CGameObject::VMethod2() { //46f460
    return 0;
}


int32_t CGameObject::VMethod3() { //46f470
    return 0;
}


int32_t CGameObject::VMethod4() { //46f480
    return 0;
}


int32_t CGameObject::VMethod5() { // 46f490
    return 0;
}


void CGameObject::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) { //46f4a0
}


void CGameObject::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) { //46f4b0
}


void CGameObject::VMethod8(int32_t arg1, int32_t arg2, int32_t arg3) { //46f4c0
}


void CGameObject::VMethod9(int32_t arg1, int32_t arg2, int32_t arg3) { //46f4d0
}


void CGameObject::VMethod10() { //46f4e0
}


int32_t CGameObject::VMethod11() { //461f73
    this->VMethod13();
    return 1;
}


void CGameObject::VMethod12() { //46f4f0
}


void CGameObject::VMethod13() { //462190
    if (this->pMapObject->my_main_unit->FUN_0041ee50(this->map_player->index) != 0 && this->scan_range != 0 && (this->screenX & 0x1f) == 0x10 && (this->screenY & 0x1f) == 0x10 && (this->x_pos != this->occupiedLocation.x || this->y_pos != this->occupiedLocation.y)) {
        this->VMethod14();
    }
}


void CGameObject::VMethod14() { //462226
    if (this->pMapObject->my_main_unit->FUN_0041ee50(this->map_player->index) != 0 && this->scan_range != 0) {
        this->pMapObject->field_0xdc = 1;

        uint16_t* land = this->pMapObject->field_0x80->GetLandscape();
        int32_t iVar6 = this->VMethod4();
        int32_t iVar2 = this->tileX;
        int32_t iVar7 = this->VMethod5();
        int32_t iVar3 = this->tileY;

        this->pMapObject->FUN_00403ca0(this);

        for (int j = 0; j < 41; j++) {
            for (int i = 0; i < 41; i++) {
                int32_t idx = iVar2 + iVar6 / 2 - 20 + j + (iVar3 + iVar7 / 2 - 20 + i) * this->pMapObject->map_width;
                if (this->pMapObject->field_0x17e4[j][i] > 0) {
                    land[idx] |= 0xc000;
                    idx++;

                    land[idx] |= 0xc000;
                    idx += this->pMapObject->map_width;

                    land[idx] |= 0xc000;
                    land[idx - 1] |= 0xc000;
                }
            }
        }

        this->occupiedLocation.x = this->x_pos;
        this->occupiedLocation.y = this->y_pos;
    }
}


int32_t CGameObject::VMethod15() { //46207a
    uint16_t* land = this->pMapObject->field_0x80->GetLandscape();

    int32_t idx = this->tileX + this->pMapObject->map_width * this->tileY;
    this->bIsBlocked = (land[idx] & 0xc000U | land[idx + 1] & 0xc000U | land[idx + this->pMapObject->map_width] & 0xc000U | land[idx + this->pMapObject->map_width + 1] & 0xc000U) != 0xc000;

    if (this->bIsBlocked == 0 || this->m_bSelected == 0) {
        return 0;
    }

    this->m_bSelected = 0;
    this->m_bSelectionDirty = 1;
    return 1;
}

void CGameObject::VMethod16() { // 46f500
}

void CGameObject::SetVals(uint16_t uni_id, int type_id, int32_t xpos, int32_t ypos, int32_t unk1, int32_t unk2, int32_t _phase, int32_t _speed, int32_t _hp) { //46187d
    this->unit_id = uni_id;
    this->x_pos = xpos;
    this->x_pos2 = xpos;
    this->y_pos = ypos;
    this->y_pos2 = ypos;
    this->z_pos = unk1;
    this->dir = unk2;
    this->phase = _phase;
    this->speed = _speed;
    this->hp_max = _hp;
    this->hp = _hp;
    this->typeId = type_id;
}





TokenEntry::~TokenEntry() { //4394bf
    if (this->mods) {
        free(this->mods);
    }
}

TokenEntry::TokenEntry() = default; //438f90


TokenEntry::TokenEntry(int arg) { //439009
    this->field_0x4 = arg;
}


TokenEntry::TokenEntry(const TokenEntry* obj) {
    //439465
    operator=(*obj);
}

TokenEntry::TokenEntry(uint8_t** data, int arg) {
    //439102
    this->slot_code = 0;
    this->field_0x14 = 0;
    this->field_0x1c = -1;
    this->field_0x20 = -1;
    this->mods_count = 0;
    this->mods_size = 0;
    this->mods = nullptr;

    uint8_t* pdata = *data;

    this->item_id = *(uint16_t*)pdata;
    pdata += 2;

    this->flg = *pdata;
    pdata++;

    ItemNamePktEntry* pkt = nullptr;
    if (this->item_id < 0xfff0) {
        if (unk_660D28.Lookup(this->item_id, *(void**)&pkt) == 0) {
            *data = pdata;
            return;
        }

        this->mods_size = pkt->data_len;
        this->mods_count = pkt->field_5;
    }

    if ((this->flg & 0x80) == 0) {
        if ((this->flg & 0x20) == 0) {
            this->count =  *(uint16_t*)pdata;
            pdata += 2;

            this->mods = malloc(this->mods_size);
            memcpy(this->mods, pkt->data, this->mods_size);
        } else {
            int esz = this->flg & 0xf;
            this->mods_count += esz + 1;
            this->mods_size += (esz + 1) * 2;
            
            int local_18 = *(uint32_t*)pdata;
            pdata += 4;

            this->mods = malloc(this->mods_size);
            memcpy(this->mods, pkt->data, this->mods_size);

            uint8_t* ot = (uint8_t*)this->mods + pkt->data_len;
            ot[0] = 0x33;
            ot[1] = 0;
            memcpy(ot + 2, pdata, esz * 2);

            pdata += esz * 2;

            memcpy((uint8_t *)this->mods + 1, &local_18, 4);

            this->flg &= 0x60;
            this->count = 1;
        }
    } else {
        this->count = this->flg & 0x3f;
        this->mods = malloc(this->mods_size);
        memcpy(this->mods, pkt->data, this->mods_size);
        this->flg &= 0x40;
    }

    if (pkt) {
        this->flg |= pkt->flags;
    }

    if (arg != 0) {
        int val = *pdata;
        pdata++;

        if (val == 0) {
            this->slot_code = 2;
        } else {
            this->slot_code = val + 4;
        }
    }

    *data = pdata;
}

void TokenEntry::operator=(const TokenEntry& obj) {
    //43a3cd
    this->field_0x4 = obj.field_0x4;
    this->item_id = obj.item_id;
    this->flg = obj.flg;
    this->mods_count = obj.mods_count;
    this->count = obj.count;
    this->field_0x14 = obj.field_0x14;
    this->slot_code = obj.slot_code;
    this->mods_size = obj.mods_size;

    if (this->mods_size == 0) {
        this->mods = nullptr;
    } else {
        this->mods = malloc(this->mods_size);
        memcpy(this->mods, obj.mods, this->mods_size);
    }

    this->field_0x1c = obj.field_0x1c;
    this->field_0x20 = obj.field_0x20;

}


const char* TokenEntry::FUN_00439973() { //439973
    char buffer[0x80];

    g_Destination[0] = '\0';

    void* name = nullptr;
    if (unk_660DA8.Lookup(this->item_id, name) == 0) {
        return g_Destination;
    }

    strcpy(g_Destination, (const char*)name);

    int32_t cast_spell_id = 0;
    int32_t value = 0;
    const uint8_t* p = (const uint8_t*)this->mods;

    for (int32_t i = 0; i < this->mods_count; i++) {
        uint8_t attr_id = *p;
        p++;

        switch (attr_id) {
        case 1:
            p += 4;
            break;

        case 0xd:
        case 0x2c:
        case 0x2d:
        case 0x2e:
        case 0x2f:
        case 0x30:
            value = *p;
            p++;

            if (strlen(txt_stats.GetLine(attr_id)) != 0) {
                sprintf(buffer, "#%s %d", txt_stats.GetLine(attr_id), value);
                strcat(g_Destination, buffer);
            }

            p++;
            value += *p;
            p++;
            sprintf(buffer, "-%d", value);
            strcat(g_Destination, buffer);
            i++;
            break;

        case 0x26:
            value = *p;
            p++;

            if (strlen(txt_stats.GetLine(attr_id)) != 0) {
                sprintf(buffer, "#%s %d", txt_stats.GetLine(attr_id), value);
                strcat(g_Destination, buffer);
            }
            break;

        case 0x29:
            value = *p;
            p++;
            cast_spell_id = value;

            if (this->GetType() == TYPE_SPELL) {
                sprintf(buffer, " %s %s%s", TxtFile::AllLines[0x5a], txt_spell.GetLine(value - 1), TxtFile::AllLines[0x5b]);
            } else {
                sprintf(buffer, "#%s %s%s", TxtFile::AllLines[0x5c], txt_spell.GetLine(value - 1), TxtFile::AllLines[0x5d]);
            }

            strcat(g_Destination, buffer);
            break;

        case 0x2a:
            value = *p;
            p++;
            sprintf(buffer, " %s %s%s", TxtFile::AllLines[0x5a], txt_spell.GetLine(value - 1), TxtFile::AllLines[0x5b]);
            strcat(g_Destination, buffer);
            break;

        case 0x32: {
            value = *p;
            p++;

            Spell spell((uint8_t)cast_spell_id);
            spell.sub_539541(value);
            if (cast_spell_id == spell::stone_curse) {
                spell.spell_power >>= 1;
            }

            CString mana_str;
            CString damage_str;
            CString power_str;
            CString misc_str;

            if (spell.damage_spread != 0) {
                damage_str.Format("#%s: %d-%d", TxtFile::AllLines[0x76], spell.damage_min, spell.damage_min + spell.damage_spread);
            }

            if (this->GetType() == TYPE_SPELL) {
                if (spell.max_range != 0) {
                    mana_str.Format("#%s: %d", TxtFile::AllLines[0x7b], spell.max_range);
                }
            } else {
                Item* item = g_GameDataRes.sub_510EE6(this->item_id);
                // Byte 0x58 of the derived object: Weapon::range / Armor::slot / Shield::protections[0]
                mana_str.Format("#%s: %d", TxtFile::AllLines[0x7b], *((uint8_t*)item + 0x58));
                delete item;
            }

            if (spell.spell_power != 0) {
                if (cast_spell_id == spell::stone_curse) {
                    power_str.Format("#%s: 0.0 -%5.1f", TxtFile::AllLines[0x7c], (double)spell.spell_power / 16.0);
                } else {
                    power_str.Format("#%s: %5.1f", TxtFile::AllLines[0x7c], (double)spell.spell_power / 16.0);
                }
            }

            uint8_t speed = spell.GetSpeed(value);
            if (speed != 0) {
                misc_str.Format("#%s: %d", TxtFile::AllLines[0xb6], speed);
            }

            uint8_t max_prob = spell.GetMaximumDamageProbability(value);
            if (max_prob != 0) {
                misc_str.Format("#%s: +%d%%", TxtFile::AllLines[0xb9], max_prob);
            }

            uint8_t min_prob = spell.GetMinimumDamageProbability(value);
            if (min_prob != 0) {
                misc_str.Format("#%s: +%d%%", TxtFile::AllLines[0xbb], min_prob);
            }

            uint8_t resist = spell.GetResistance(value);
            if (resist != 0) {
                misc_str.Format("#%s: +%d", TxtFile::AllLines[0xb7], resist);
            }

            uint8_t rays = spell.GetRays(value);
            if (rays != 0) {
                misc_str.Format("#%s: %d", TxtFile::AllLines[0xba], rays);
            }

            int16_t sight = spell.GetSight(value);
            if (sight != 0) {
                misc_str.Format("#%s: %d", TxtFile::AllLines[0xb8], sight);
            }

            strcat(g_Destination, (LPCTSTR)damage_str);
            strcat(g_Destination, (LPCTSTR)mana_str);
            strcat(g_Destination, (LPCTSTR)power_str);
            strcat(g_Destination, (LPCTSTR)misc_str);
            break;
        }

        case 0x33:
            value = *p;
            p++;

            if (this->GetType() != TYPE_SPELL) {
                strcat(g_Destination, "#");
                strcat(g_Destination, TxtFile::AllLines[0xbd]);
            }
            break;

        default:
            value = *p;
            p++;

            if (strlen(txt_stats.GetLine(attr_id)) != 0) {
                sprintf(buffer, "#%s %+d", txt_stats.GetLine(attr_id), value);
                strcat(g_Destination, buffer);
            }

            if (attr_id == 0xb || attr_id == 8) {
                strcat(g_Destination, "%");
            }
            break;
        }
    }

    return g_Destination;
}


int32_t TokenEntry::sub_4396FB() { //4396fb
    char str[0x50];

    strcpy(str, txt_heropicture.GetLine(this->GetId() - 1));

    if (strcmp(str, "bowman") == 0) {
        return 1;
    }
    if (strcmp(str, "archer") == 0) {
        return 1;
    }
    if (strcmp(str, "xbowman") == 0) {
        return 1;
    }
    if (strcmp(str, "axeman2h") == 0) {
        return 1;
    }
    if (strcmp(str, "swordsman2h") == 0) {
        return 1;
    }
    if (strcmp(str, "mage_st") == 0) {
        return 1;
    }

    return 0;
}


int32_t TokenEntry::sub_43A5E5() { //43a5e5
    if (this->GetType() != TYPE_SPELL) {
        return 0;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    MagicItem& item = g_GameDataRes.magic_items[this->GetShape() << 5 | this->GetId()];

    if (item.name.Find("Crown") != -1) {
        return 1;
    }
    if (item.name.Find("Documents") != -1) {
        return 1;
    }
    if (item.name.Find("Treasure") == -1 || main_wnd->sessionMode == 2) {
        return 0;
    }

    return 1;
}


int32_t TokenEntry::sub_43A6D5() { //43a6d5
    if (this->GetType() != TYPE_SPELL) {
        return 0;
    }

    MagicItem& item = g_GameDataRes.magic_items[this->GetShape() << 5 | this->GetId()];
    return item.name == "Quest Treasure";
}


int32_t TokenEntry::sub_4A7880(int32_t num) { //4a7880
    this->count -= num;

    if ((int32_t)this->count > 0) {
        return 1;
    }

    this->count += num;
    return 0;
}


int32_t TokenEntry::FUN_004a78c0() { //4a78c0
    if (this->FUN_0041f0d0() == 0 && this->count == 0) {
        return 1;
    }

    return 0;
}


int TokenEntry::FUN_004396d6() { //4396d6
    void* ptr = nullptr;
    return unk_660DA8.Lookup(this->item_id, ptr);
}


int32_t TokenEntry::FUN_0041f0d0() { //41f0d0
    return this->item_id == 0xffff;
}


int32_t TokenEntry::sub_4A7850(int32_t num) { //4a7850
    this->count += num;
    return 1;
}


CString TokenEntry::FUN_004394f3() { //4394f3
    char buffer[0x50];

    if (this->GetType() == TYPE_SPELL) {
        sprintf(buffer, "%02d%02d%03d", this->GetMaterial(), this->GetType(), this->GetShape() << 5 | this->GetId());
    } else {
        sprintf(buffer, "%02d%02d%1d%02d", this->GetMaterial(), this->GetType(), this->GetShape(), this->GetId());
    }

    return CString(buffer);
}


int32_t TokenEntry::sub_4A7900(TokenEntry* other) { //4a7900
    if (this == other) {
        return 1;
    }

    if (this->item_id != other->item_id) {
        return 0;
    }

    if (this->slot_code != other->slot_code
        && (this->slot_code < 5 || this->slot_code > 8
            || other->slot_code < 5 || other->slot_code > 8)) {
        return 0;
    }

    if ((this->flg & 0x20) != 0 || (other->flg & 0x20) != 0) {
        return 0;
    }

    return 1;
}


int32_t TokenEntry::sub_43988E() { //43988e
    const uint8_t* p = (const uint8_t*)this->mods;

    for (int32_t i = 0; i < this->mods_count; i++) {
        uint8_t attr_id = *p;
        p++;

        switch (attr_id) {
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 0xa:
        case 0xb:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x19:
        case 0x1b:
        case 0x1c:
        case 0x1d:
        case 0x1e:
        case 0x1f:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
            return attr_id;
        }

        if (attr_id == 1) {
            p += 4;
        } else {
            p += 1;
        }
    }

    return 0;
}



CUnit::CUnit() { //46411c
    this->mp_max = 0;
    this->mp = 0;
    this->lastVoicePlaybackTick = 0;
    this->equipmentTokens.fill(nullptr);
    this->sprite = nullptr;
    this->sprite_b = nullptr;
    this->heroSpritePictureName[0] = 0;
    this->y_pos2 = 0;
    this->x_pos2 = 0;
    this->unitFlags = 0;
    this->death_state = 0;
    this->heroSpriteArmorMaterial = -1;
    this->field_0x161 = 0;
    this->field_0x162 = 0;
    this->field_0x164 = 0;
    this->serverId = 0;
    this->questFlags = -1;
    this->field_0x156 = 0;
}

CUnit::~CUnit() { //46459d
    for (TokenEntry* eq : this->equipmentTokens) {
        if (eq) {
            delete eq;
        }
    }

    if (this->sprite) {
        delete this->sprite;
    }

    if (this->sprite_b) {
        delete this->sprite_b;
    }
}

void CUnit::VMethod30(const char* str, CGameBitmap* bmp1, CGameBitmap* bmp2) { // 469ffc
    if (bmp1 != nullptr && bmp1->GetData() != nullptr) {
        memset(bmp1->GetData(), 0, bmp1->GetWidth(0) * bmp1->GetHeight(0) * 2);
    }

    if (bmp2 != nullptr && bmp2->GetData() != nullptr) {
        memset(bmp2->GetData(), 0, bmp2->GetWidth(0) * bmp2->GetHeight(0));
    }

    char sprite_prefix[256] = { 0 };
    strcpy(sprite_prefix, "graphics\\equipment\\");

    switch (this->unitFlags & 6) {
    case 0:
        strcat(sprite_prefix, "mfighter\\");
        break;
    case 2:
        strcat(sprite_prefix, "mmage\\");
        break;
    case 4:
        strcat(sprite_prefix, "ffighter\\");
        break;
    case 6:
        strcat(sprite_prefix, "fmage\\");
        break;
    default:
        break;
    }

    CSprite256* item_sprites[12] = { nullptr };
    CSprite256* extra_sprites[12] = { nullptr };

    for (int i = 0; i < 12; ++i) {
        if (this->equipmentTokens[i] != nullptr) {
            CString base_name = sprite_prefix;
            CString extra_name = sprite_prefix;
            CString item_name = this->equipmentTokens[i]->FUN_004394f3();
            CString item_name2 = this->equipmentTokens[i]->FUN_004394f3();

            base_name += "primary\\";
            base_name += item_name;
            base_name += ".256";

            extra_name += "secondary\\";
            extra_name += item_name2;
            extra_name += ".256";

            item_sprites[i] = new CSprite256(base_name);
            item_sprites[i]->ResetPalette(1, 1, 0);

            if (i == 3 || i == 8 || i == 9 || (i == 7 && (this->unitFlags & 2) != 0)) {
                extra_sprites[i] = new CSprite256(extra_name);
                if (extra_sprites[i]) {
                    extra_sprites[i]->ResetPalette(1, 1, 0);
                }
            }
        }
    }

    char sprite_name[256] = { 0 };
    sprintf(sprite_name, "%s%d.256", sprite_prefix, this->face);

    CSprite256* unit_sprite = new CSprite256(sprite_name);
    if (unit_sprite) {
        unit_sprite->ResetPalette(1, 1, 0);
    }

    CBmp64* horse_sprite = nullptr;
    if (0x10 < this->typeId && this->typeId < 0x16) {
        horse_sprite = new CBmp64("graphics\\infowindow\\horse.bmp");
    }

    if (bmp1 != nullptr) {
        bmp1->SelectBitmapForDraw();
    }

    if (horse_sprite != nullptr) {
        horse_sprite->VMethod2(0, 0, 0, 0, 0);
    }

    if ((this->unitFlags & 1) != 0 && (this->unitFlags & 2) == 0) {
        if ((this->unitFlags & 4) == 0) {
            if (g_spr_backm != nullptr) {
                g_spr_backm->VMethod2(0, 0, 0, 0, 0);
            }
        } else if (g_spr_backf != nullptr) {
            g_spr_backf->VMethod2(0, 0, 0, 0, 0);
        }
    }

    int first_equipment_state = 0;
    if (this->equipmentTokens[0] != nullptr) {
        first_equipment_state = this->equipmentTokens[0]->FUN_004396d6();
    }

    if ((this->unitFlags & 2) == 0) {
        if (unit_sprite != nullptr) {
            unit_sprite->VMethod2(0, 0, 0, 0, 0);
        }

        if (item_sprites[11] != nullptr) {
            item_sprites[11]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[10] != nullptr) {
            item_sprites[10]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[6] != nullptr) {
            item_sprites[6]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[3] != nullptr) {
            item_sprites[3]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[4] != nullptr) {
            item_sprites[4]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[8] != nullptr) {
            item_sprites[8]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[9] != nullptr) {
            item_sprites[9]->VMethod2(0, 0, 0, 0, 0);
        }
        if (first_equipment_state == 0 && item_sprites[0] != nullptr) {
            item_sprites[0]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[7] != nullptr) {
            item_sprites[7]->VMethod2(0, 0, 0, 0, 0);
        }
        if (extra_sprites[3] != nullptr) {
            extra_sprites[3]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[5] != nullptr) {
            item_sprites[5]->VMethod2(0, 0, 0, 0, 0);
        }
        if (extra_sprites[8] != nullptr) {
            extra_sprites[8]->VMethod2(0, 0, 0, 0, 0);
        }
        if (extra_sprites[9] != nullptr) {
            extra_sprites[9]->VMethod2(0, 0, 0, 0, 0);
        }
        if (first_equipment_state == 0) {
            if (item_sprites[1] != nullptr) {
                item_sprites[1]->VMethod2(0, 0, 0, 0, 0);
            }
        } else if (item_sprites[0] != nullptr) {
            item_sprites[0]->VMethod2(0, 0, 0, 0, 0);
        }

        if (bmp2 != nullptr) {
            bmp2->SelectBitmapForDraw();

            if (item_sprites[11] != nullptr) {
                item_sprites[11]->VMethod12(0, 0, 0, 12);
            }
            if (item_sprites[10] != nullptr) {
                item_sprites[10]->VMethod12(0, 0, 0, 11);
            }
            if (item_sprites[6] != nullptr) {
                item_sprites[6]->VMethod12(0, 0, 0, 7);
            }
            if (item_sprites[3] != nullptr) {
                item_sprites[3]->VMethod12(0, 0, 0, 4);
            }
            if (item_sprites[4] != nullptr) {
                item_sprites[4]->VMethod12(0, 0, 0, 5);
            }
            if (item_sprites[8] != nullptr) {
                item_sprites[8]->VMethod12(0, 0, 0, 9);
            }
            if (item_sprites[9] != nullptr) {
                item_sprites[9]->VMethod12(0, 0, 0, 10);
            }
            if (first_equipment_state == 0 && item_sprites[0] != nullptr) {
                item_sprites[0]->VMethod12(0, 0, 0, 1);
            }
            if (item_sprites[7] != nullptr) {
                item_sprites[7]->VMethod12(0, 0, 0, 8);
            }
            if (extra_sprites[3] != nullptr) {
                extra_sprites[3]->VMethod12(0, 0, 0, 4);
            }
            if (item_sprites[5] != nullptr) {
                item_sprites[5]->VMethod12(0, 0, 0, 6);
            }
            if (extra_sprites[8] != nullptr) {
                extra_sprites[8]->VMethod12(0, 0, 0, 9);
            }
            if (extra_sprites[9] != nullptr) {
                extra_sprites[9]->VMethod12(0, 0, 0, 10);
            }
            if (first_equipment_state == 0) {
                if (item_sprites[1] != nullptr) {
                    item_sprites[1]->VMethod12(0, 0, 0, 2);
                }
            } else if (item_sprites[0] != nullptr) {
                item_sprites[0]->VMethod12(0, 0, 0, 1);
            }
        }
    } else {
        if (item_sprites[7] != nullptr) {
            item_sprites[7]->VMethod2(0, 0, 0, 0, 0);
        }
        unit_sprite->VMethod2(0, 0, 0, 0, 0);
        if (item_sprites[11] != nullptr) {
            item_sprites[11]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[9] != nullptr) {
            item_sprites[9]->VMethod2(0, 0, 0, 0, 0);
        }
        if (extra_sprites[9] != nullptr) {
            extra_sprites[9]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[3] != nullptr) {
            item_sprites[3]->VMethod2(0, 0, 0, 0, 0);
        }
        if (extra_sprites[3] != nullptr) {
            extra_sprites[3]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[6] != nullptr) {
            item_sprites[6]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[4] != nullptr) {
            item_sprites[4]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[8] != nullptr) {
            item_sprites[8]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[0] != nullptr) {
            item_sprites[0]->VMethod2(0, 0, 0, 0, 0);
        }
        if (item_sprites[5] != nullptr) {
            item_sprites[5]->VMethod2(0, 0, 0, 0, 0);
        }
        if (extra_sprites[7] != nullptr) {
            extra_sprites[7]->VMethod2(0, 0, 0, 0, 0);
        }

        if (bmp2 != nullptr) {
            bmp2->SelectBitmapForDraw();

            if (item_sprites[7] != nullptr) {
                item_sprites[7]->VMethod12(0, 0, 0, 8);
            }
            unit_sprite->VMethod12(0, 0, 0, 0);
            if (item_sprites[11] != nullptr) {
                item_sprites[11]->VMethod12(0, 0, 0, 8);
            }
            if (item_sprites[9] != nullptr) {
                item_sprites[9]->VMethod12(0, 0, 0, 12);
            }
            if (extra_sprites[9] != nullptr) {
                extra_sprites[9]->VMethod12(0, 0, 0, 10);
            }
            if (item_sprites[3] != nullptr) {
                item_sprites[3]->VMethod12(0, 0, 0, 4);
            }
            if (extra_sprites[3] != nullptr) {
                extra_sprites[3]->VMethod12(0, 0, 0, 4);
            }
            if (item_sprites[6] != nullptr) {
                item_sprites[6]->VMethod12(0, 0, 0, 7);
            }
            if (item_sprites[4] != nullptr) {
                item_sprites[4]->VMethod12(0, 0, 0, 5);
            }
            if (item_sprites[8] != nullptr) {
                item_sprites[8]->VMethod12(0, 0, 0, 9);
            }
            if (item_sprites[0] != nullptr) {
                item_sprites[0]->VMethod12(0, 0, 0, 1);
            }
            if (item_sprites[5] != nullptr) {
                item_sprites[5]->VMethod12(0, 0, 0, 6);
            }
            if (extra_sprites[7] != nullptr) {
                extra_sprites[7]->VMethod12(0, 0, 0, 8);
            }
        }
    }

    if (str != nullptr) {
        bmp1->VMethod7();
        static_cast<CBmp64*>(bmp1)->WriteFile(str, bmp2); // TODO: is `bmp1` actually `CBmp64*`?
    }

    uint8_t palette_data[256 * 4] = { 0 };
    for (int i = 0; i < 0x100; ++i) {
        palette_data[i * 4 + 0] = ((uint8_t)i & 0xF) * 8 - 0x80;
        palette_data[i * 4 + 1] = ((uint8_t)i & 0xF) * 8 - 0x80;
        palette_data[i * 4 + 2] = ((uint8_t)i & 0xF) * 8 - 0x80;
    }

    if (bmp2 != nullptr) {
        bmp2->palette_data = palette_data;
        bmp2->ResetPalette(1, 1, 0);
    }

    for (int i = 0; i < 12; ++i) {
        delete item_sprites[i];
        item_sprites[i] = nullptr;

        delete extra_sprites[i];
        extra_sprites[i] = nullptr;
    }

    if (unit_sprite != nullptr) {
        delete unit_sprite;
    }

    if (str != nullptr) {
        this->unitFlags &= ~0x8;
    }

    if (this->action == 3 || this->action == 7 || this->action == 8) {
        this->action = 0;
        this->phase = 0;
        this->field_0x34 = -1;
        this->action_phase = 0;
        this->action_segments = 0;
    }

    if (horse_sprite != nullptr) {
        delete horse_sprite;
    }
}


void CUnit::CopyFromUnit(const Unit& uni) { //46b0d7
    this->typeId = uni.typeId;
    this->ApplyFace(uni.face);
    this->body = uni.body;
    this->mind = uni.mind;
    this->reaction = uni.reaction;
    this->spirit = uni.spirit;
    this->hp = uni.hp;
    this->hp_max = uni.hp_max;
    this->mp = uni.mp;
    this->mp_max = uni.mp_max;
    this->attack = uni.hit_values.attack;
    this->defence = uni.protections.defense;
    this->absorb = uni.protections.absorption;
    this->hand_dmg_min = uni.hit_values.hand_damage_min;
    this->hand_dmg_spread = uni.hit_values.hand_damage_spread;
    this->speed = uni.speed;
    this->scan_range = uni.scan_range;
    this->serverId = uni.server_id;
    this->carrying_weight_100g = uni.carrying_weight_100g;

    if (((Unit&)uni).VMethod8() == 0) { //FIXME when this method become const
        this->skill_levels[0] = uni.protections.weapon_protections[1];
        this->skill_levels[1] = uni.protections.weapon_protections[2];
        this->skill_levels[2] = uni.protections.weapon_protections[3];
        this->skill_levels[3] = uni.protections.weapon_protections[4];
        this->skill_levels[4] = uni.protections.weapon_protections[5];
    } else {
        Humanoid& hum = (Humanoid&)uni;
        this->skill_levels[0] = hum.hit_values.skill_levels[1];
        this->skill_levels[1] = hum.hit_values.skill_levels[2];
        this->skill_levels[2] = hum.hit_values.skill_levels[3];
        this->skill_levels[3] = hum.hit_values.skill_levels[4];
        this->skill_levels[4] = hum.hit_values.skill_levels[5];

        this->experience_per_sphere[0] = hum.experience_per_sphere[0];
        this->experience_per_sphere[1] = hum.experience_per_sphere[1];
        this->experience_per_sphere[2] = hum.experience_per_sphere[2];
        this->experience_per_sphere[3] = hum.experience_per_sphere[3];
        this->experience_per_sphere[4] = hum.experience_per_sphere[4];

        this->exp_summary = this->experience_per_sphere[0] + this->experience_per_sphere[1] + this->experience_per_sphere[2] + this->experience_per_sphere[3] + this->experience_per_sphere[4];

        PacketUnitStateVec* local_20 = &PacketUnitStateVec::Inst;
        local_20->entry_count = 0;
        local_20->start_index = 0;
        local_20->data_size = 0;

        if (uni.weapon) {
            uni.weapon->StoreToPacket(local_20, 0);
        } else {
            Item itm;
            itm.StoreToPacket(local_20, 0);
        }

        if (uni.shield) {
            uni.shield->StoreToPacket(local_20, 0);
        } else {
            Item itm;
            itm.StoreToPacket(local_20, 0);
        }

        for (int i = 3; i < 13; i++) {
            if (hum.equipment[i]) {
                hum.equipment[i]->StoreToPacket(local_20, 0);
            } else {
                Item itm;
                itm.StoreToPacket(local_20, 0);
            }
        }

        uint8_t* pdata = local_20->data;
        for (int i = 0; i < 12; i++) {
            if (this->equipmentTokens[i]) {
                delete this->equipmentTokens[i];
            }

            TokenEntry* te = new TokenEntry(&pdata, 0);
            if (te->item_id == 0) {
                delete te;
                this->equipmentTokens[i] = nullptr;
            } else {
                te->slot_code = 1;
                this->equipmentTokens[i] = te;
            }
        }
    }

    this->magic_protect[0] = uni.protections.magic_protections[1];
    this->magic_protect[1] = uni.protections.magic_protections[2];
    this->magic_protect[2] = uni.protections.magic_protections[3];
    this->magic_protect[3] = uni.protections.magic_protections[4];
    this->magic_protect[4] = uni.protections.magic_protections[5];

    if (this->typeId == 0x49 || this->typeId == 0x68 || this->typeId == 0x46) {
        this->field_0x156 = 2;
    } else {
        this->field_0x156 = 0;
    }

    this->ReloadSprite();
}

void CUnit::ApplyFace(int32_t _face) { //46b7d2

    this->face = _face;

    if (this->typeId > 0x1f && this->typeId < 0x40) {
        this->unitFlags &= 0x80;
        this->unitFlags |= 1 | 8;

        this->typeId -= 0x21;

        if ((this->typeId & 1) != 0) {
            this->unitFlags |= 4;
        }

        if ((this->typeId & 2) != 0) {
            this->unitFlags |= 2;
        }

        this->typeId = 1;
    } else if (this->typeId < 0x1a) {
        this->unitFlags &= 0x80;
        this->unitFlags |= 0x10 | 8;

        if ((this->face & 0x80) != 0) {
            this->unitFlags |= 4;
        }

        this->face &= ~0x80;

        if (this->typeId > 0x16 && this->typeId < 0x19) {
            this->unitFlags |= 2;
        }
    }
}

void CUnit::ReloadSprite() { // 46b91c
    if ((this->unitFlags & 1) != 0) {
        CString sname;

        if (this->action == 6) {
            sname = "unarmed";
            if ((this->unitFlags & 2) != 0 /* && strcmp(local_c8, "unarmed") == 0 */) {
                sname = "mage_st";
            }
        } else {
            int32_t hero_id = 0;

            if (this->equipmentTokens[0] != nullptr) {
                hero_id = this->equipmentTokens[0]->GetId() - 1;
            }

            sname = txt_heropicture.GetLine(hero_id);

            if (this->equipmentTokens[1] != NULL) {
                sname += "_";
            }

            if ((this->unitFlags & 2) != 0 && sname == "unarmed") {
                sname = "mage";
            }
        }

        bool do_reload = sname != this->heroSpritePictureName;

        if ((this->unitFlags & 2) == 0) {
            if (this->equipmentTokens[7] == nullptr) {
                if (this->heroSpriteArmorMaterial != -1) {
                    do_reload = true;
                }
            } else if (this->heroSpriteArmorMaterial == -1) {
                do_reload = true;
            }
        }

        if (do_reload) {
            if (this->sprite) {
                delete this->sprite;
            }

            if (this->sprite_b) {
                delete this->sprite_b;
            }

            CString sprite_name = "graphics\\units\\";

            if (this->unitFlags & 2) {
                sprite_name += "heroes";
            } else if (this->equipmentTokens[7] == nullptr) {
                this->heroSpriteArmorMaterial = -1;
                sprite_name += "heroes_l";
            } else {
                sprite_name += g_CUnitMaterialSpritePaths[this->equipmentTokens[7]->GetMaterial()];
                this->heroSpriteArmorMaterial = this->equipmentTokens[7]->GetMaterial();
            }

            sprite_name += "\\";
            sprite_name += sname;

            CString sprite_name_b = sprite_name;

            sprite_name += "\\sprites.256";
            sprite_name_b += "\\spritesb.256";

            this->sprite = new CSprite256(sprite_name);
            this->sprite_b = new CSprite256(sprite_name_b);

            strcpy(this->heroSpritePictureName, sname);

            if (strcmp(this->heroSpritePictureName, "unarmed") == 0) {
                this->typeId = 1;
            } else if (strcmp(this->heroSpritePictureName, "unarmed_") == 0) {
                this->typeId = 2;
            } else if (strcmp(this->heroSpritePictureName, "swordsman") == 0) {
                this->typeId = 3;
            } else if (strcmp(this->heroSpritePictureName, "swordsman_") == 0) {
                this->typeId = 4;
            } else if (strcmp(this->heroSpritePictureName, "swordsman2h") == 0) {
                this->typeId = 5;
            } else if (strcmp(this->heroSpritePictureName, "axeman") == 0) {
                this->typeId = 7;
            } else if (strcmp(this->heroSpritePictureName, "axeman_") == 0) {
                this->typeId = 8;
            } else if (strcmp(this->heroSpritePictureName, "axeman2h") == 0) {
                this->typeId = 9;
            } else if (strcmp(this->heroSpritePictureName, "clubman") == 0) {
                this->typeId = 10;
            } else if (strcmp(this->heroSpritePictureName, "clubman_") == 0) {
                this->typeId = 11;
            } else if (strcmp(this->heroSpritePictureName, "pikeman") == 0) {
                this->typeId = 12;
            } else if (strcmp(this->heroSpritePictureName, "pikeman_") == 0) {
                this->typeId = 13;
            } else if (strcmp(this->heroSpritePictureName, "archer") == 0) {
                this->typeId = 14;
            } else if (strcmp(this->heroSpritePictureName, "bowman") == 0) {
                this->typeId = 14;
            } else if (strcmp(this->heroSpritePictureName, "xbowman") == 0) {
                this->typeId = 15;
            } else if (strcmp(this->heroSpritePictureName, "mage") == 0) {
                this->typeId = 23;
            } else if (strcmp(this->heroSpritePictureName, "mage_st") == 0) {
                this->typeId = 24;
            }
        }
    }
}

void CGameObject::FUN_0046246b(int32_t left, int32_t right, int32_t y, int32_t fill_width, uint16_t color_lo, uint16_t color_mid, uint16_t color_hi) { // 46246B
    BigStruct2* map = this->pMapObject;
    if (map->show_hp != 0 && this->IsSelected() == 0) {
        ShadowRect(CRect(left + 4, y - 2, left + fill_width + 4, y + 2), 8);
        FUN_00457aa6(left + 4, y - 2, left + fill_width + 4, y - 1, color_hi);
        FUN_00457aa6(left + 4, y - 1, left + fill_width + 4, y, color_lo);
        FUN_00457aa6(left + 4, y, left + fill_width + 4, y + 1, color_mid);
        FUN_00457aa6(left + 4, y + 1, left + fill_width + 4, y + 2, color_hi);
        return;
    }

    FillRectColorSimple(left + 4, y - 2, right - 4, y + 2, GetColorRGB(0x40, 0x40, 0x40));
    FillRectColorSimple(left + 4, y - 1, right - 4, y, GetColorRGB(0x80, 0x80, 0x80));
    FillRectColorSimple(left + 4, y, right - 4, y + 1, GetColorRGB(0x60, 0x60, 0x60));
    FillRectColorSimple(left + 4, y - 2, left + fill_width + 4, y - 1, color_hi);
    FillRectColorSimple(left + 4, y - 1, left + fill_width + 4, y, color_lo);
    FillRectColorSimple(left + 4, y, left + fill_width + 4, y + 1, color_mid);
}

void CGameObject::FUN_004627bb(int32_t x, int32_t y) { // 4627BB
    BigStruct2* map = this->pMapObject;
    int32_t group = this->GetControlGroup();
    if (group < 0) {
        return;
    }

    char buf[20];
    sprintf(buf, "%d", group);

    uint16_t* shadow = clrsh_ShockingBlack;
    if (map->show_hp != 0 && this->IsSelected() == 0) {
        shadow = clrsh_TechBlack;
    }
    g_font3->DrawTextWithShadow(x, y, buf, 0, shadow, 1);
}

void CUnit::VMethod8(int32_t arg1, int32_t arg2, int32_t arg3) { // 466a78
    if (this->death_state >= 5) {
        return;
    }

    BigStruct2* map = this->pMapObject;

    if (this->FUN_00462405(0x20) >= 0 && map->my_main_unit->FUN_0041ee50(this->map_player->index) == 0) {
        return;
    }

    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];
    int32_t left = this->centerScreenX - vfx->center_x + vfx->selection.left;
    int32_t top = this->centerScreenY - vfx->center_y - this->terrainHeightOffset + vfx->selection.top - this->z_pos;
    int32_t bottom = this->centerScreenY - vfx->center_y - this->terrainHeightOffset + vfx->selection.bottom - this->z_pos;
    int32_t right = this->centerScreenX - vfx->center_x + vfx->selection.right;

    int32_t y = top;

    gfx_ball->VMethod10(left, y - 2, 0, 0, 4, 4);
    gfx_ball->VMethod10(right - 4, y - 2, 0, 0, 4, 4);

    uint16_t color_lo;
    uint16_t color_mid;
    uint16_t color_hi;
    if (this->hp < this->hp_max / 4) {
        color_lo = GetColorRGB(0xFF, 0, 0);
        color_mid = GetColorRGB(0xC0, 0, 0);
        color_hi = GetColorRGB(0x80, 0, 0);
    } else if (this->hp < this->hp_max / 2) {
        color_lo = GetColorRGB(0xFF, 0xFF, 0);
        color_mid = GetColorRGB(0xC0, 0xC0, 0);
        color_hi = GetColorRGB(0x80, 0x80, 0);
    } else {
        color_lo = GetColorRGB(0, 0xFF, 0);
        color_mid = GetColorRGB(0, 0xC0, 0);
        color_hi = GetColorRGB(0, 0x80, 0);
    }

    uint16_t add_mask = g_ColorAddMask;
    if (map->show_hp != 0 && this->IsSelected() == 0) {
        color_lo = (color_lo >> 1) & add_mask;
        color_mid = (color_mid >> 1) & add_mask;
        color_hi = (color_hi >> 1) & add_mask;
    }

    int32_t fill = this->hp * (right - left - 8) / this->hp_max;
    if (fill == 0 && this->hp != 0) {
        fill = 1;
    }
    this->FUN_0046246b(left, right, y, fill, color_lo, color_mid, color_hi);

    if (this->mp_max > 0) {
        y += 4;
        color_lo = GetColorRGB(0, 0, 0xFF);
        color_mid = GetColorRGB(0, 0, 0xC0);
        color_hi = GetColorRGB(0, 0, 0x80);
        if (map->show_hp != 0 && this->IsSelected() == 0) {
            color_lo = (color_lo >> 1) & add_mask;
            color_mid = (color_mid >> 1) & add_mask;
            color_hi = (color_hi >> 1) & add_mask;
        }

        int32_t mp_fill = this->mp * (right - left - 8) / this->mp_max;
        if (mp_fill == 0) {
            mp_fill = 1;
        }

        gfx_ball->VMethod10(left, y - 2, 0, 0, 4, 4);
        gfx_ball->VMethod10(right - 4, y - 2, 0, 0, 4, 4);
        this->FUN_0046246b(left, right, y, mp_fill, color_lo, color_mid, color_hi);
    }

    this->FUN_004627bb(left, y + 4);

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    int32_t session_mode = main_wnd->sessionMode;
    if (session_mode == 2 || g_settings.ClanNames == 0 || this->str1[0] == 0 || (this->unitFlags & 1) == 0) {
        if (session_mode != 2 && g_settings.ClanNames != 0) {
            int32_t quest_flag = 0;
            POSITION pos = map->field_0x4970->quests_map.GetStartPosition();
            while (pos != nullptr) {
                uint32_t quest_id;
                Quest* quest;
                map->field_0x4970->quests_map.GetNextAssoc(pos, quest_id, quest);

                if (quest->IsInWork() != 0) {
                    if (this->unit_id == quest->GetObj()) {
                        if (quest->Kind() != 3 && quest->Kind() != 0xC) {
                            quest_flag = 1;
                        }
                    } else if (this->typeId == (quest->GetObj() & 0xFF) && this->face == (quest->GetObj() >> 8)) {
                        if (quest->Kind() == 2) {
                            quest_flag = 1;
                        }
                    } else if (this->questFlags == quest->GetObj()) {
                        if (quest->Kind() == 3 || quest->Kind() == 0xC) {
                            quest_flag = 1;
                        }
                    } else if (this->map_player->index == quest->GetObj()) {
                        if (quest->Kind() == 0xD) {
                            quest_flag = 1;
                        }
                    }
                }
            }

            if (quest_flag != 0) {
                g_font2->DrawTxt((left + right) / 2, y - 0xF, TxtFile::AllLines[0x15A], 2, g_colors_human_pals[this->map_player->color]);
                g_font2->DrawTxt((left + right) / 2, y - 0x19, TxtFile::AllLines[0x159], 2, g_colors_human_pals[this->map_player->color]);
            }
        }
        return;
    }

    if (this->str2[0] != 0) {
        g_font2->DrawTxt((left + right) / 2, y - 0xF, this->str2, 2, g_colors_human_pals[this->map_player->color]);
        g_font2->DrawTxt((left + right) / 2, y - 0x19, this->str1, 2, g_colors_human_pals[this->map_player->color]);
    } else {
        g_font2->DrawTxt((left + right) / 2, y - 0xF, this->str1, 2, g_colors_human_pals[this->map_player->color]);
    }
}

void CUnit::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) { // 464974
    if (this->death_state >= 5) {
        return;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    BigStruct2* map = this->pMapObject;

    int32_t sun_angle = (int32_t)(std::tan(this->pMapObject->field_0x80->FUN_004a7b79()) * 65536.0);
    (void)sun_angle; // computed but never used in the original binary

    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];
    int32_t file_idx = vfx->file;
    int32_t palette_id = vfx->palette;
    int32_t flip = 0;
    int32_t base_phases = vfx->move_phases + vfx->move_begin_phases;
    int32_t dir8 = ((this->dir - 8) & 0xE) >> 1;
    if (vfx->flip != 0 && dir8 > 4) {
        dir8 = 8 - dir8;
        flip = 1;
    }

    int32_t vfx_idx = this->typeId;

    for (int32_t i = 0; i < this->transientVisualElements.GetSize(); i++) {
        GO_11c* elem = &this->transientVisualElements[i];
        if (elem->z > 0) {
            ProjectileInfo* proj = g_ProjectileInfos[elem->vfx_type];
            int32_t x = this->centerScreenX + elem->x - proj->width / 2;
            int32_t y = this->centerScreenY - this->terrainHeightOffset - this->z_pos + elem->y - proj->height / 2 - elem->z;
            proj->FUN_0041f8f0()->VMethod2(x, y, elem->phase, 0, 0);
        }
    }

    int32_t frame = 0;
    switch (this->last_action) {
    case 0:
        if (vfx->idle_phases != 0) {
            frame = dir8 * vfx->idle_phases + (base_phases + vfx->attack_phases + vfx->dying_phases) * 8 + 0x10 + vfx->idle_anim_frames[this->phase];
            if (vfx->flip != 0 && dir8 > 4) {
                dir8 = 8 - dir8;
                flip = 1;
            }
            if (vfx->flip != 0) {
                frame = (base_phases + vfx->attack_phases + vfx->dying_phases) * 5 + dir8 * vfx->idle_phases + 9 + vfx->idle_anim_frames[this->phase];
            }
        } else if (this->death_state == 1) {
            vfx_idx = vfx->dying;
            UnitVFXUnfo* dying_vfx = g_VFX_info[vfx_idx];
            file_idx = dying_vfx->file;
            base_phases = dying_vfx->move_phases + dying_vfx->move_begin_phases;
            dir8 = ((this->dir - 8) & 0xE) >> 1;
            if (dying_vfx->flip != 0 && dir8 > 4) {
                dir8 = 8 - dir8;
                flip = 1;
            }
            if (dying_vfx->flip != 0) {
                frame = (base_phases + dying_vfx->attack_phases) * 5 + (dir8 + 1) * dying_vfx->dying_phases + 8;
            } else {
                frame = (dir8 + 1) * dying_vfx->dying_phases + (base_phases + dying_vfx->attack_phases) * 8 + 0xF;
            }
        } else if (this->death_state <= 1 || g_VFX_info[vfx->dying]->bone_phases == 0) {
            frame = (this->dir - 8) & 0xF;
            if (vfx->flip != 0 && frame > 8) {
                frame = 0x10 - frame;
                flip = 1;
            }
        } else {
            vfx_idx = vfx->dying;
            UnitVFXUnfo* dying_vfx = g_VFX_info[vfx_idx];
            file_idx = dying_vfx->file;
            base_phases = dying_vfx->move_phases + dying_vfx->move_begin_phases;
            dir8 = ((this->dir - 8) & 0xE) >> 1;
            if (dying_vfx->flip != 0 && dir8 > 4) {
                dir8 = 8 - dir8;
                flip = 1;
            }
            if (dying_vfx->flip != 0) {
                frame = (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 5 + dir8 * dying_vfx->bone_phases + 9 + this->death_state - 2;
            } else {
                frame = dir8 * dying_vfx->bone_phases + (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 8 + 0x10 + this->death_state - 2;
            }
        }
        break;
    case 1:
        if (vfx->flip != 0) {
            frame = dir8 * base_phases + vfx->move_begin_phases + 9 + vfx->move_anim_frames[this->phase % vfx->move_anim_frame_cnt];
        } else {
            frame = dir8 * base_phases + vfx->move_begin_phases + 0x10 + vfx->move_anim_frames[this->phase % vfx->move_anim_frame_cnt];
        }
        break;
    case 3:
    case 7:
    case 8:
        if (vfx->flip != 0) {
            frame = base_phases * 5 + dir8 * vfx->attack_phases + 9 + vfx->attack_anim_frames[this->phase];
        } else {
            frame = dir8 * vfx->attack_phases + base_phases * 8 + 0x10 + vfx->attack_anim_frames[this->phase];
        }
        break;
    case 5:
        frame = (this->dir - 8) & 0xF;
        if (vfx->flip != 0 && frame > 8) {
            frame = 0x10 - frame;
            flip = 1;
        }
        break;
    case 6: {
        vfx_idx = vfx->dying;
        UnitVFXUnfo* dying_vfx = g_VFX_info[vfx_idx];
        file_idx = dying_vfx->file;
        base_phases = dying_vfx->move_phases + dying_vfx->move_begin_phases;
        dir8 = ((this->dir - 8) & 0xE) >> 1;
        if (dying_vfx->flip != 0 && dir8 > 4) {
            dir8 = 8 - dir8;
            flip = 1;
        }
        if (dying_vfx->flip != 0) {
            frame = (base_phases + dying_vfx->attack_phases) * 5 + dir8 * dying_vfx->dying_phases + 9 + this->phase / 2;
        } else {
            frame = dir8 * dying_vfx->dying_phases + (base_phases + dying_vfx->attack_phases) * 8 + 0x10 + this->phase / 2;
        }
        break;
    }
    }

    if (this->FUN_00462405(0x2C) >= 0 && this->death_state <= 2) {
        frame = (this->dir - 8) & 0xF;
        if (vfx->flip != 0 && frame > 8) {
            frame = 0x10 - frame;
            flip = 1;
        }
    }

    CGamePalette tmp_palette;
    CGamePalette* palette;
    if (palette_id == 0) {
        palette = g_Human_palettes[this->map_player->color];
    } else if (palette_id == 1) {
        palette = g_VFX_info[vfx_idx]->FUN_0046f680(0);
    } else {
        palette = g_VFX_info[vfx_idx]->FUN_0046f680(this->face - 1);
    }

    if (this->FUN_00462405(0x2C) >= 0 && this->death_state <= 2) {
        if (palette_id == 0) {
            palette = g_Human_palettes[0x10];
        } else {
            palette = &tmp_palette;
            tmp_palette.SetPalette((RGBQUAD*)g_VFX_info[vfx_idx]->FUN_0046f6e0(0), 0x10, 5, 0);
        }
    }

    if (map->field_0x994 == this && main_wnd->drag_entry == NULL) {
        arg3 = 0;
    }

    UnitVFXUnfo* draw_vfx = g_VFX_info[vfx_idx];
    if ((this->unitFlags & 1) != 0) {
        if (this->sprite->GetFrameCount() <= frame) {
            return;
        }
        int32_t cx = draw_vfx->center_x - draw_vfx->width / 2 + this->sprite->GetWidth(frame) / 2;
        int32_t cy = draw_vfx->center_y - draw_vfx->height / 2 + this->sprite->GetHeight(frame) / 2;
        int32_t x = this->centerScreenX - cx;
        int32_t y = this->centerScreenY - cy - this->terrainHeightOffset - this->z_pos;
        if (this->FUN_00462405(0x20) < 0) {
            this->sprite->VMethod1(x, y, frame, arg3, palette, flip);
        } else {
            if (map->my_main_unit->FUN_0041ee50(this->map_player->index) == 0) {
                return;
            }
            this->sprite->VMethod9(x, y, frame, arg3, palette, flip);
        }
        cx = draw_vfx->center_x - draw_vfx->width / 2 + this->sprite_b->GetWidth(frame) / 2;
        cy = draw_vfx->center_y - draw_vfx->height / 2 + this->sprite_b->GetHeight(frame) / 2;
        x = this->centerScreenX - cx;
        y = this->centerScreenY - cy - this->terrainHeightOffset - this->z_pos;
        if (g_settings.Smoothing != 0) {
            this->sprite_b->VMethod9(x, y, frame, arg3, palette, flip);
        }
    } else {
        this->IsKindOf(RUNTIME_CLASS(CAirUnit)); // result unused in the original binary
        UnitGfxFile* gfx_file = g_UnitGfxFiles[file_idx];
        int32_t cx = draw_vfx->center_x - draw_vfx->width / 2 + gfx_file->FUN_0046f700()->GetWidth(frame) / 2;
        int32_t cy = draw_vfx->center_y - draw_vfx->height / 2 + gfx_file->FUN_0046f700()->GetHeight(frame) / 2;
        int32_t x = this->centerScreenX - cx;
        int32_t y = this->centerScreenY - cy - this->terrainHeightOffset - this->z_pos;
        if (this->FUN_00462405(0x20) < 0) {
            gfx_file->FUN_0046f700()->VMethod1(x, y, frame, arg3, palette, flip);
        } else {
            if (map->my_main_unit->FUN_0041ee50(this->map_player->index) == 0) {
                return;
            }
            gfx_file->FUN_0046f700()->VMethod9(x, y, frame, arg3, palette, flip);
        }
        cx = draw_vfx->center_x - draw_vfx->width / 2 + gfx_file->FUN_0046f730()->GetWidth(frame) / 2;
        cy = draw_vfx->center_y - draw_vfx->height / 2 + gfx_file->FUN_0046f730()->GetHeight(frame) / 2;
        x = this->centerScreenX - cx;
        y = this->centerScreenY - cy - this->terrainHeightOffset - this->z_pos;
        if (g_settings.Smoothing != 0 || this->typeId == 0x45) {
            gfx_file->FUN_0046f730()->VMethod9(x, y, frame, arg3, palette, flip);
        }
    }

    for (int32_t i = 0; i < this->transientVisualElements.GetSize(); i++) {
        GO_11c* elem = &this->transientVisualElements[i];
        if (elem->z <= 0) {
            ProjectileInfo* proj = g_ProjectileInfos[elem->vfx_type];
            int32_t x = this->centerScreenX + elem->x - proj->width / 2;
            int32_t y = this->centerScreenY - this->terrainHeightOffset - this->z_pos + elem->y - proj->height / 2 - elem->z;
            proj->FUN_0041f8f0()->VMethod2(x, y, elem->phase, 0, 0);
        }
    }
}

void CUnit::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) { // 465b5a
    if (this->death_state >= 5) {
        return;
    }

    BigStruct2* map = this->pMapObject;

    int32_t sun_angle = (int32_t)(std::tan(map->field_0x80->FUN_004a7b79()) * 65536.0);

    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];
    int32_t file_idx = vfx->file;
    int32_t flip = 0;
    int32_t base_phases = vfx->move_phases + vfx->move_begin_phases;
    int32_t dir8 = ((this->dir - 8) & 0xE) >> 1;
    if (vfx->flip != 0 && dir8 > 4) {
        dir8 = 8 - dir8;
        flip = 1;
    }

    int32_t vfx_idx = this->typeId;

    int32_t frame = 0;
    switch (this->last_action) {
    case 0:
        if (vfx->idle_phases != 0) {
            frame = dir8 * vfx->idle_phases + (base_phases + vfx->attack_phases + vfx->dying_phases) * 8 + 16 + vfx->idle_anim_frames[this->phase];
            if (vfx->flip != 0 && dir8 > 4) {
                dir8 = 8 - dir8;
                flip = 1;
            }
            if (vfx->flip != 0) {
                frame = (base_phases + vfx->attack_phases + vfx->dying_phases) * 5 + dir8 * vfx->idle_phases + 9 + vfx->idle_anim_frames[this->phase];
            }
        } else if (this->death_state == 1) {
            vfx_idx = vfx->dying;
            UnitVFXUnfo* dying_vfx = g_VFX_info[vfx_idx];
            file_idx = dying_vfx->file;
            base_phases = dying_vfx->move_phases + dying_vfx->move_begin_phases;
            dir8 = ((this->dir - 8) & 0xE) >> 1;
            if (dying_vfx->flip != 0 && dir8 > 4) {
                dir8 = 8 - dir8;
                flip = 1;
            }
            if (dying_vfx->flip != 0) {
                frame = (base_phases + dying_vfx->attack_phases) * 5 + (dir8 + 1) * dying_vfx->dying_phases + 8;
            } else {
                frame = (dir8 + 1) * dying_vfx->dying_phases + (base_phases + dying_vfx->attack_phases) * 8 + 15;
            }
        } else if (this->death_state <= 1 || g_VFX_info[vfx->dying]->bone_phases == 0) {
            frame = (this->dir - 8) & 0xF;
            if (vfx->flip != 0 && frame > 8) {
                frame = 16 - frame;
                flip = 1;
            }
        } else {
            vfx_idx = vfx->dying;
            UnitVFXUnfo* dying_vfx = g_VFX_info[vfx_idx];
            file_idx = dying_vfx->file;
            base_phases = dying_vfx->move_phases + dying_vfx->move_begin_phases;
            dir8 = ((this->dir - 8) & 0xE) >> 1;
            if (dying_vfx->flip != 0 && dir8 > 4) {
                dir8 = 8 - dir8;
                flip = 1;
            }
            if (dying_vfx->flip != 0) {
                frame = (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 5 + dir8 * dying_vfx->bone_phases + 7 + this->death_state;
            } else {
                frame = dir8 * dying_vfx->bone_phases + (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 8 + 14 + this->death_state;
            }
        }
        break;
    case 1:
        if (vfx->flip != 0) {
            frame = dir8 * base_phases + vfx->move_begin_phases + 9 + vfx->move_anim_frames[this->phase % vfx->move_anim_frame_cnt];
        } else {
            frame = dir8 * base_phases + vfx->move_begin_phases + 16 + vfx->move_anim_frames[this->phase % vfx->move_anim_frame_cnt];
        }
        break;
    case 3:
    case 7:
    case 8:
        if (vfx->flip != 0) {
            frame = base_phases * 5 + dir8 * vfx->attack_phases + 9 + vfx->attack_anim_frames[this->phase];
        } else {
            frame = dir8 * vfx->attack_phases + base_phases * 8 + 16 + vfx->attack_anim_frames[this->phase];
        }
        break;
    case 5:
        frame = (this->dir - 8) & 0xF;
        if (vfx->flip != 0 && frame > 8) {
            frame = 16 - frame;
            flip = 1;
        }
        break;
    case 6: {
        vfx_idx = vfx->dying;
        UnitVFXUnfo* dying_vfx = g_VFX_info[vfx_idx];
        file_idx = dying_vfx->file;
        base_phases = dying_vfx->move_phases + dying_vfx->move_begin_phases;
        dir8 = ((this->dir - 8) & 0xE) >> 1;
        if (dying_vfx->flip != 0 && dir8 > 4) {
            dir8 = 8 - dir8;
            flip = 1;
        }
        if (dying_vfx->flip != 0) {
            frame = (base_phases + dying_vfx->attack_phases) * 5 + dir8 * dying_vfx->dying_phases + 9 + this->phase / 2;
        } else {
            frame = dir8 * dying_vfx->dying_phases + (base_phases + dying_vfx->attack_phases) * 8 + 16 + this->phase / 2;
        }
        break;
    }
    }

    if (this->FUN_00462405(0x2C) >= 0 && this->death_state <= 2) {
        frame = (this->dir - 8) & 0xF;
        if (vfx->flip != 0 && frame > 8) {
            frame = 16 - frame;
            flip = 1;
        }
    }

    UnitVFXUnfo* draw_vfx = g_VFX_info[vfx_idx];
    if ((this->unitFlags & 1) != 0) {
        if (this->sprite->GetFrameCount() <= frame) {
            return;
        }
        int32_t shadow_offset = (int32_t)(std::tan(map->field_0x80->FUN_004a7b79()) * (this->sprite->GetHeight(frame) / 2 + draw_vfx->height / 2 - draw_vfx->center_y));
        int32_t cx = draw_vfx->center_x - draw_vfx->width / 2 + this->sprite->GetWidth(frame) / 2;
        int32_t cy = draw_vfx->center_y - draw_vfx->height / 2 + this->sprite->GetHeight(frame) / 2;
        int32_t x = this->centerScreenX - cx - shadow_offset;
        int32_t y = this->centerScreenY - cy - this->terrainHeightOffset;
        if (this->FUN_00462405(0x20) < 0) {
            this->sprite->VMethod11(x, y, frame, g_DeltaCLR.shade_delta, sun_angle, flip);
        } else {
            if (map->my_main_unit->FUN_0041ee50(this->map_player->index) == 0) {
                return;
            }
            this->sprite->VMethod11(x, y, frame, g_DeltaCLR.field_10, sun_angle, flip);
        }
        cx = draw_vfx->center_x - draw_vfx->width / 2 + this->sprite_b->GetWidth(frame) / 2;
        cy = draw_vfx->center_y - draw_vfx->height / 2 + this->sprite_b->GetHeight(frame) / 2;
        x = this->centerScreenX - cx - shadow_offset;
        y = this->centerScreenY - cy - this->terrainHeightOffset;
        if (this->FUN_00462405(0x20) < 0 && g_settings.Smoothing != 0) {
            this->sprite_b->VMethod11(x, y, frame, g_DeltaCLR.field_10, sun_angle, flip);
        }
    } else {
        UnitGfxFile* gfx_file = g_UnitGfxFiles[file_idx];
        int32_t shadow_offset = (int32_t)(std::tan(map->field_0x80->FUN_004a7b79()) * (gfx_file->FUN_0046f700()->GetHeight(frame) / 2 + draw_vfx->height / 2 - draw_vfx->center_y));
        int32_t is_air = this->IsKindOf(RUNTIME_CLASS(CAirUnit));
        int32_t cx = draw_vfx->center_x - draw_vfx->width / 2 + gfx_file->FUN_0046f700()->GetWidth(frame) / 2;
        int32_t cy = draw_vfx->center_y - draw_vfx->height / 2 + gfx_file->FUN_0046f700()->GetHeight(frame) / 2;
        int32_t x = this->centerScreenX - cx - shadow_offset;
        int32_t y = this->centerScreenY - cy - this->terrainHeightOffset;
        if (this->FUN_00462405(0x20) < 0) {
            if (is_air) {
                gfx_file->FUN_0046f700()->VMethod3(x + shadow_offset + sun_angle / 2000, y, frame, g_DeltaCLR.shade_delta, flip);
            } else {
                gfx_file->FUN_0046f700()->VMethod11(x, y, frame, g_DeltaCLR.shade_delta, sun_angle, flip);
            }
        } else {
            if (map->my_main_unit->FUN_0041ee50(this->map_player->index) == 0) {
                return;
            }
            if (is_air) {
                gfx_file->FUN_0046f700()->VMethod3(x + shadow_offset + sun_angle / 2000, y, frame, g_DeltaCLR.field_10, flip);
            } else {
                gfx_file->FUN_0046f700()->VMethod11(x, y, frame, g_DeltaCLR.field_10, sun_angle, flip);
            }
        }
        cx = draw_vfx->center_x - draw_vfx->width / 2 + gfx_file->FUN_0046f730()->GetWidth(frame) / 2;
        cy = draw_vfx->center_y - draw_vfx->height / 2 + gfx_file->FUN_0046f730()->GetHeight(frame) / 2;
        x = this->centerScreenX - cx - shadow_offset;
        y = this->centerScreenY - cy - this->terrainHeightOffset;
        if (this->FUN_00462405(0x20) < 0 && g_settings.Smoothing != 0) {
            if (is_air) {
                gfx_file->FUN_0046f730()->VMethod3(x + shadow_offset + sun_angle / 2000, y, frame, g_DeltaCLR.field_10, flip);
            } else {
                gfx_file->FUN_0046f730()->VMethod11(x, y, frame, g_DeltaCLR.field_10, sun_angle, flip);
            }
        }
    }
}

// 4cb0a0
TransientVisualEffectBuilder::TransientVisualEffectBuilder() {
    this->visual_elements.RemoveAll();
}

// 4cb190
TransientVisualEffectBuilder::~TransientVisualEffectBuilder() {
    this->visual_elements.RemoveAll();
}

// 46f390
void TransientVisualEffectBuilder::FUN_0046f390() {
    this->arc_x.RemoveAll();
    this->arc_y.RemoveAll();
    this->visual_elements.RemoveAll();
}

// 4cced2
void TransientVisualEffectBuilder::FUN_004CCED2(int16_t height, uint8_t phase) {
    this->visual_elements.Add(GO_11c(0, -height, 0, 0x14, phase));
}

// 4cce04
void TransientVisualEffectBuilder::FUN_004CCE04(int16_t height, uint8_t effect_type, uint8_t phase) {
    int16_t x_offset = 0;
    int16_t y_offset = 0;
    switch (effect_type) {
    case 0x10:
        x_offset = 6;
        break;
    case 0x18:
        x_offset = -6;
        break;
    case 0x22:
        y_offset = 6;
        break;
    case 0x2e:
        y_offset = -6;
        break;
    }
    this->visual_elements.Add(GO_11c(-x_offset, -height - y_offset, 0, effect_type, phase));
}

// 4cd4b0
double FUN_004CD4B0(int32_t degrees) {
    return degrees * (3.141592653589793 / 180.0);
}

// 4ccaf2
void TransientVisualEffectBuilder::FUN_004CCAF2(int16_t height, float radius, uint32_t phase) {
    for (int32_t angle = 0x59; angle > 0; angle -= 0x12) {
        double rad = FUN_004CD4B0(angle + (phase & 0xFF) * 0x12);
        int16_t x = (int16_t)(std::cos(rad) * radius);
        int16_t z = (int16_t)(std::sin(rad) * radius);
        uint8_t anim_phase = 4 - angle / 0x12;
        this->visual_elements.Add(GO_11c(x, -height, z, 0x30, anim_phase));
        this->visual_elements.Add(GO_11c(z, -height, -x, 0x30, anim_phase));
        this->visual_elements.Add(GO_11c(-x, -height, -z, 0x30, anim_phase));
        this->visual_elements.Add(GO_11c(-z, -height, x, 0x30, anim_phase));
    }
}

// 4cc798
void TransientVisualEffectBuilder::FUN_004CC798(int32_t x, int32_t y, int32_t height, float speed, float radius, int32_t angle) {
    float y_scale = speed / radius;
    double rad = FUN_004CD4B0(angle);
    double cos_a = std::cos(rad);
    double sin_a = std::sin(rad);
    double anim_scale = 5.0 / speed;

    this->FUN_004CC5D1(x, y, (int32_t)radius, 4);

    for (int32_t i = 0; i < this->circle_points.GetSize(); i++) {
        CPoint* pt = &this->circle_points[i];
        int16_t cos_x = (int16_t)(pt->x * cos_a);
        int16_t sin_x = (int16_t)(pt->x * sin_a);
        int16_t y_pos = -(int16_t)(pt->y * y_scale);
        uint8_t anim_phase = std::abs(4 - (int32_t)(std::abs(pt->y) * anim_scale));
        this->visual_elements.Add(GO_11c(x + cos_x, y + y_pos, height + sin_x, 0x3E, anim_phase));
        this->visual_elements.Add(GO_11c(x - sin_x, y + y_pos, height + cos_x, 0x3E, anim_phase));
    }
}

// 4cc5d1
void TransientVisualEffectBuilder::FUN_004CC5D1(int32_t x, int32_t y, int32_t radius, int32_t step) {
    int32_t px = 0;
    int32_t py = radius;
    int32_t decision = (1 - radius) * 2;
    int32_t counter = step;
    this->circle_points.RemoveAll();

    while (py > 0) {
        if (counter >= step) {
            this->circle_points.Add(CPoint(x + px, y + py));
            this->circle_points.Add(CPoint(x - px, y + py));
            this->circle_points.Add(CPoint(x + px, y - py));
            this->circle_points.Add(CPoint(x - px, y - py));
            counter = 0;
        }
        counter += 1;
        if (decision < 0) {
            int32_t threshold = (decision + py) * 2 - 1;
            if (threshold <= 0) {
                px += 1;
                decision = decision + px * 2 + 1;
            } else {
                px += 1;
                py -= 1;
                decision = decision + (px - py) * 2 + 2;
            }
        } else if (decision == 0) {
            px += 1;
            py -= 1;
            decision = decision + (px - py) * 2 + 2;
        } else {
            int32_t threshold = (decision - px) * 2 - 1;
            if (threshold <= 0) {
                px += 1;
                py -= 1;
                decision = decision + (px - py) * 2 + 2;
            } else {
                py -= 1;
                decision = decision + 1 - py * 2;
            }
        }
    }
}

// 4cb85f
void TransientVisualEffectBuilder::FUN_004CB85F(double px0, double py0, double px1, double py1, double px2, double py2) {
    double x0_sq = px0 * px0;
    double x1_sq = px1 * px1;
    double x2_sq = px2 * px2;
    double det = (px2 - px1) * x0_sq + (px0 - px2) * x1_sq + (px1 - px0) * x2_sq;
    double quad = -((py2 - py1) * px0 + (py0 - py2) * px1 + (py1 - py0) * px2) / det;
    double lin = ((py2 - py1) * x0_sq + (py0 - py2) * x1_sq + (py1 - py0) * x2_sq) / det;
    double cst = -((px1 * py2 - px2 * py1) * x0_sq + (px2 * py0 - px0 * py2) * x1_sq + (px0 * py1 - px1 * py0) * x2_sq) / det;

    for (double t = px0; t < px2; t += 6.0) {
        double y = (quad * t + lin) * t + cst;
        this->arc_x.Add(t);
        this->arc_y.Add(y);
    }
}

// 4cb993
void TransientVisualEffectBuilder::FUN_004CB993(double x0, double y0, double x1, double y1) {
    double y_tolerance = this->line_length * 0.15;
    double dx = x1 - x0;
    double dy = y1 - y0;
    double y_accum = 0.0;
    double x_accum = 0.0;
    CArray<double> xs;
    CArray<double> ys;

    this->FUN_0046f390();
    this->wobble_scale = this->line_length * 0.03;

    double dir;
    if (std::rand() & 1) {
        dir = 1.0;
    } else {
        dir = -1.0;
    }

    this->jitter_progress.RemoveAll();
    this->jitter_wobble.RemoveAll();
    this->jitter_progress.Add(0.0);
    this->jitter_wobble.Add(0.0);

    do {
        double jitter_x = (double)(std::rand() % 7) * dir;
        double jitter_y = (double)(std::rand() % 50) * 0.01;
        if (std::fabs(jitter_x) <= 2.0 || jitter_y <= 0.15) {
            continue;
        }
        x_accum += jitter_x;
        if (x_accum > 3.0) {
            x_accum = 3.0;
        }
        if (x_accum < -3.0) {
            x_accum = -3.0;
        }
        this->jitter_wobble.Add(x_accum);
        y_accum += jitter_y;
        this->jitter_progress.Add(y_accum);
        dir = -dir;
    } while (y_accum < 0.7);

    if (this->jitter_progress.GetAt(this->jitter_progress.GetUpperBound()) >= 1.0) {
        this->jitter_progress.RemoveAt(this->jitter_progress.GetUpperBound());
        this->jitter_wobble.RemoveAt(this->jitter_wobble.GetUpperBound());
    }

    this->jitter_progress.Add(1.0);
    this->jitter_wobble.Add(0.0);

    xs.Add(x0);
    ys.Add(y0);

    int32_t n = 0;
    double px = this->jitter_progress[1] * dx + x0;
    xs.Add(px);
    double py = this->jitter_progress[1] * dy + y0;
    py = this->jitter_wobble[1] * this->wobble_scale + py;
    ys.Add(py);
    n = 1;

    for (int32_t i = 2; i < this->jitter_progress.GetSize() - 1; i++) {
        px = this->jitter_progress[i] * dx + x0;
        py = this->jitter_progress[i] * dy + y0;
        py = this->jitter_wobble[i] * this->wobble_scale + py;
        double mid_x = (px - xs[n]) / 2.0 + xs[n];
        xs.Add(mid_x);
        double mid_y = (py - ys[n]) / 2.0 + ys[n];
        ys.Add(mid_y);
        n++;
        xs.Add(px);
        ys.Add(py);
        n++;
    }

    xs.Add(x1);
    ys.Add(y1);

    for (int32_t i = 2; i < xs.GetSize(); i += 2) {
        this->FUN_004CB85F(xs[i - 2], ys[i - 2], xs[i - 1], ys[i - 1], xs[i], ys[i]);
    }

    for (int32_t i = 0; i < this->arc_x.GetSize(); i++) {
        if (std::fabs(this->arc_y[i] - y0) > y_tolerance) {
            this->FUN_004CB993(x0, y0, x1, y1);
            break;
        }
    }
}

// 4cc059
void TransientVisualEffectBuilder::FUN_004cc059(double x0, double y0, double x1, double y1, uint8_t arg5) {
    double dx = x1 - x0;
    double dy = y1 - y0;
    this->line_length = std::hypot(dx, dy);

    this->FUN_004CB993(x0, y0, x0 + this->line_length, y0);

    double step_x = dx / this->line_length;
    double step_y = dy / this->line_length;

    for (int32_t i = 0; i < this->arc_x.GetSize(); i++) {
        double px_f = (this->arc_x[i] - this->arc_x[0]) * step_x + this->arc_x[0];
        int32_t px = (int32_t)(px_f - (this->arc_y[i] - this->arc_y[0]) * step_y);
        double py_f = (this->arc_x[i] - this->arc_x[0]) * step_y + this->arc_y[0];
        int32_t py = (int32_t)((this->arc_y[i] - this->arc_y[0]) * step_x + py_f);
        this->visual_elements.Add(GO_11c((int16_t)px, (int16_t)py, 0, arg5, 0));
    }
}

// 4cc22d
int32_t TransientVisualEffectBuilder::FUN_004CC22D(CArray<GO_11c>* existing, int16_t x_radius, int16_t y_radius, int16_t z_radius, uint16_t visual_id) {
    int16_t age_step = (int16_t)(z_radius / 7.0 + 1.0);
    for (int32_t i = 0; i < existing->GetSize(); i++) {
        GO_11c* elem = &(*existing)[i];
        if (elem->vfx_type == 0x38 && elem->phase < 7) {
            elem->phase += 1;
            elem->y -= age_step;
            this->visual_elements.Add(*elem);
        }
    }

    if (visual_id < 8) {
        return 0;
    }

    int32_t count = std::rand() % 3 + 3;
    for (int32_t i = 0; i < count; i++) {
        double rad = FUN_004CD4B0(std::rand() % 360);
        GO_11c elem;
        elem.x = (int16_t)(std::cos(rad) * x_radius);
        elem.y = 0;
        elem.z = (int16_t)(std::sin(rad) * y_radius);
        elem.vfx_type = 0x38;
        elem.phase = 0;
        this->visual_elements.Add(elem);
    }
    return 1;
}

// 4cc3fd
int32_t TransientVisualEffectBuilder::FUN_004CC3FD(CArray<GO_11c>* existing, int16_t x_radius, int16_t y_radius, int16_t z_radius, uint16_t visual_id) {
    int16_t age_step = (int16_t)(z_radius / 7.0 + 1.0);
    for (int32_t i = 0; i < existing->GetSize(); i++) {
        GO_11c* elem = &(*existing)[i];
        if (elem->vfx_type == 0x3C && elem->phase < 7) {
            elem->phase += 1;
            elem->y += age_step;
            this->visual_elements.Add(*elem);
        }
    }

    if (visual_id < 8) {
        return 0;
    }

    int32_t count = std::rand() % 3 + 3;
    for (int32_t i = 0; i < count; i++) {
        double rad = FUN_004CD4B0(std::rand() % 0x168);
        GO_11c elem;
        elem.x = (int16_t)(std::cos(rad) * x_radius);
        elem.y = -z_radius;
        elem.z = (int16_t)(std::sin(rad) * y_radius);
        elem.vfx_type = 0x3C;
        elem.phase = 0;
        this->visual_elements.Add(elem);
    }
    return 1;
}

// 4cc944
void TransientVisualEffectBuilder::FUN_004CC944(int16_t height, float speed, float radius, uint32_t phase) {
    this->FUN_004CC798(0, 0, height, speed, radius, (phase & 0xFF) << 2);

    int32_t y_offset = (int32_t)std::fabs(speed * ((int32_t)(phase & 0xFF) * 0.02222222222222222 - 1.0));
    int32_t span = (int32_t)(std::sin(std::acos(y_offset / (double)speed)) * radius);
    uint8_t anim_phase = 4 - std::abs((int32_t)(phase & 0xFF) - 0x2D) / 9;

    this->FUN_004CC5D1(0, 0, span, anim_phase * 2);

    for (int32_t i = 0; i < this->circle_points.GetSize(); i++) {
        CPoint* pt = &this->circle_points[i];
        int16_t x = pt->x;
        int16_t z = pt->y;
        int16_t y_top = -(int16_t)(height + y_offset);
        this->visual_elements.Add(GO_11c(x, y_top, z, 0x3E, anim_phase));
        int16_t y_bottom = -(int16_t)(height - y_offset);
        this->visual_elements.Add(GO_11c(x, y_bottom, z, 0x3E, anim_phase));
    }
}

// 4ccc7b
void TransientVisualEffectBuilder::FUN_004CCC7B(int16_t height, float radius, uint32_t phase) {
    for (int32_t angle = 0; angle < 0x5A; angle += 0x12) {
        double rad = FUN_004CD4B0(angle + (phase & 0xFF) * 0x12);
        int16_t x = (int16_t)(std::cos(rad) * radius);
        int16_t z = (int16_t)(std::sin(rad) * radius);
        uint8_t anim_phase = 4 - angle / 0x12;
        this->visual_elements.Add(GO_11c(x, -height, z, 0x40, anim_phase));
        this->visual_elements.Add(GO_11c(z, -height, -x, 0x40, anim_phase));
        this->visual_elements.Add(GO_11c(-x, -height, -z, 0x40, anim_phase));
        this->visual_elements.Add(GO_11c(-z, -height, x, 0x40, anim_phase));
    }
}

// 4cb269
int __cdecl CompareVisualElements(const void* left, const void* right) {
    const GO_11c* lhs = (const GO_11c*)left;
    const GO_11c* rhs = (const GO_11c*)right;

    if (lhs->y < rhs->y) {
        return 1;
    }
    if (rhs->y < lhs->y) {
        return -1;
    }
    return 0;
}

void CUnit::VMethod16() { // 468dcb
    CArray<GO_11c> visual_elements;

    for (int32_t i = 0; i < this->effect_timers.GetSize();) {
        uint32_t effect = this->effect_timers[i];
        uint16_t effect_timer = (uint16_t)effect;
        uint32_t effect_type = effect >> 16;

        switch (effect_type) {
        case 0x10:
        case 0x18:
        case 0x22:
        case 0x2e: {
            TransientVisualEffectBuilder builder;
            builder.FUN_004CCE04(this->VMethod4() << 5, effect_type, effect_timer % 6);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        case 0x14: {
            TransientVisualEffectBuilder builder;
            builder.FUN_004CCED2(this->VMethod4() << 5, effect_timer % 6);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        case 0x30: {
            TransientVisualEffectBuilder builder;
            builder.FUN_004CCAF2(this->VMethod4() << 5, 20.0f, effect_timer % 5);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        case 0x38: {
            TransientVisualEffectBuilder builder;
            int16_t z_radius = this->VMethod4() << 5;
            int16_t y_radius = this->VMethod4() << 3;
            int16_t x_radius = this->VMethod4() << 4;
            builder.FUN_004CC22D(&this->transientVisualElements, x_radius, y_radius, z_radius, effect_timer);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        case 0x3c: {
            TransientVisualEffectBuilder builder;
            int16_t z_radius = this->VMethod4() << 5;
            int16_t y_radius = this->VMethod4() << 3;
            int16_t x_radius = this->VMethod4() << 4;
            builder.FUN_004CC3FD(&this->transientVisualElements, x_radius, y_radius, z_radius, effect_timer);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        case 0x3e: {
            TransientVisualEffectBuilder builder;
            uint32_t phase = effect_timer % 90;
            float radius = this->VMethod4() << 4;
            float speed = this->VMethod4() * 28;
            int16_t height = this->VMethod4() * 11;
            builder.FUN_004CC944(height, speed, radius, phase);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        case 0x40: {
            TransientVisualEffectBuilder builder;
            builder.FUN_004CCC7B(this->VMethod4() << 5, 20.0f, effect_timer % 5);
            visual_elements.Append(builder.visual_elements);
            break;
        }
        }

        uint32_t remaining = (uint32_t)effect_timer - 1;
        this->effect_timers[i] = (effect & 0xffff0000) | remaining;
        if (remaining == 0) {
            this->effect_timers.RemoveAt(i, 1);
        } else {
            i++;
        }
    }

    if (visual_elements.GetSize() != 0) {
        std::qsort(visual_elements.GetData(), visual_elements.GetSize(), sizeof(GO_11c), CompareVisualElements);
    }
    this->transientVisualElements.Copy(visual_elements);
}

int32_t CUnit::VMethod11() { // 46785f
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];

    if (this->death_state >= 5) {
        return 1;
    }

    this->VMethod16();

    if (this->action_segments == 0) {
        if (this->last_action != 0 || this->effect_timers.GetSize() != 0 || g_VFX_info[this->typeId]->idle_phases > 0) {
            this->m_bSelectionDirty = 1;
        }
        this->last_action = 0;
        if (vfx->idle_phases == 0) {
            this->field_0x34 = -1;
            this->phase = 0;
            this->action_phase = 0;
        } else {
            this->action_phase = this->action_phase + 1;
            this->phase = this->action_phase % vfx->idle_anim_frame_cnt;
        }
        this->VMethod13();
        this->x_pos2 = this->x_pos;
        this->y_pos2 = this->y_pos;
        return 1;
    }

    this->m_bSelectionDirty = 1;
    switch (this->action) {
    case 1: {
        int32_t dx = this->action_x / this->action_segments;
        int32_t dy = this->action_y / this->action_segments;
        this->action_x -= dx;
        this->move_accum_x += std::abs(dx);
        this->action_y -= dy;
        this->move_accum_y += std::abs(dy);
        this->x_pos += dx;
        this->y_pos += dy;
        this->dir = this->action_dir;
        this->action_phase = (int32_t)(std::sqrt((double)(dx * dx) + (double)(dy * dy) + 0.0) + this->action_phase);

        if (this->typeId >= 0) {
            this->x_pos2 = this->x_pos;
            this->y_pos2 = this->y_pos;
            this->phase = this->action_phase / 16;
            break;
        }

        switch (this->dir) {
        case 0:
        case 8: {
            int32_t sway = this->dir == 8 ? 8 : -8;
            switch ((this->move_accum_y / 26) & 7) {
            case 0:
                if (this->field_0x34 != 0) {
                    this->field_0x34 = 0;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 1:
                if (this->field_0x34 != 1) {
                    this->field_0x34 = 1;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 2:
                if (this->field_0x34 != 2) {
                    this->field_0x34 = 2;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 3:
                if (this->field_0x34 != 3) {
                    this->field_0x34 = 3;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 4:
                if (this->field_0x34 != 4) {
                    this->field_0x34 = 4;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos - sway;
                }
                break;
            case 5:
                if (this->field_0x34 != 5) {
                    this->field_0x34 = 5;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos + sway;
                }
                break;
            case 6:
                if (this->field_0x34 != 6) {
                    this->field_0x34 = 6;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 7:
                if (this->field_0x34 != 7) {
                    this->field_0x34 = 7;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            }
            break;
        }
        case 4:
        case 12: {
            int32_t sway = this->dir == 4 ? 8 : -8;
            switch ((this->move_accum_x / 32) & 7) {
            case 0:
                if (this->field_0x34 != 0) {
                    this->field_0x34 = 0;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 1:
                if (this->field_0x34 != 1) {
                    this->field_0x34 = 1;
                    this->x_pos2 = this->x_pos - sway;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 2:
                if (this->field_0x34 != 2) {
                    this->field_0x34 = 2;
                    this->x_pos2 = this->x_pos - sway;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 3:
                if (this->field_0x34 != 3) {
                    this->field_0x34 = 3;
                    this->x_pos2 = this->x_pos + sway;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 4:
                if (this->field_0x34 != 4) {
                    this->field_0x34 = 4;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 5:
                if (this->field_0x34 != 5) {
                    this->field_0x34 = 5;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 6:
                if (this->field_0x34 != 6) {
                    this->field_0x34 = 6;
                    this->x_pos2 = this->x_pos + sway * 2;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 7:
                if (this->field_0x34 != 7) {
                    this->field_0x34 = 7;
                    this->x_pos2 = this->x_pos + sway;
                    this->y_pos2 = this->y_pos;
                }
                break;
            }
            break;
        }
        default: {
            int32_t sway_x = this->dir < 8 ? 8 : -8;
            int32_t sway_y = (this->dir == 6 || this->dir == 10) ? 8 : -8;
            switch ((this->move_accum_x / 25) & 7) {
            case 0:
                if (this->field_0x34 != 0) {
                    this->field_0x34 = 0;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 1:
                if (this->field_0x34 != 1) {
                    this->field_0x34 = 1;
                    this->x_pos2 = this->x_pos - sway_x;
                    this->y_pos2 = this->y_pos - sway_y;
                }
                break;
            case 2:
                if (this->field_0x34 != 2) {
                    this->field_0x34 = 2;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 3:
                if (this->field_0x34 != 3) {
                    this->field_0x34 = 3;
                    this->x_pos2 = this->x_pos + sway_x * 2;
                    this->y_pos2 = this->y_pos + sway_y * 2;
                }
                break;
            case 4:
                if (this->field_0x34 != 4) {
                    this->field_0x34 = 4;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 5:
                if (this->field_0x34 != 5) {
                    this->field_0x34 = 5;
                    this->x_pos2 = this->x_pos;
                    this->y_pos2 = this->y_pos;
                }
                break;
            case 6:
                if (this->field_0x34 != 6) {
                    this->field_0x34 = 6;
                    this->x_pos2 = this->x_pos + sway_x;
                    this->y_pos2 = this->y_pos + sway_y;
                }
                break;
            case 7:
                if (this->field_0x34 != 7) {
                    this->field_0x34 = 7;
                    this->x_pos2 = this->x_pos + sway_x;
                    this->y_pos2 = this->y_pos + sway_y;
                }
                break;
            }
            break;
        }
        }
        this->phase = this->field_0x34;
        break;
    }
    case 3:
        if (this->action_phase == vfx->attack_delay) {
            this->VMethod21();
        }
        this->dir = this->action_dir;
        this->phase = this->action_phase;
        this->action_phase = this->action_phase + 1;
        this->x_pos2 = this->x_pos;
        this->y_pos2 = this->y_pos;
        break;
    case 5: {
        int32_t turn = this->action_dir * 16 - this->turn_angle;
        if (turn < -0x7F) {
            turn += 0x100;
        }
        if (turn > 0x80) {
            turn -= 0x100;
        }
        this->turn_angle += turn / this->action_segments;
        if (this->turn_angle < 0) {
            this->turn_angle += 0x100;
        }
        this->dir = this->turn_angle >> 4;
        this->action_phase = this->action_phase + 1;
        this->x_pos2 = this->x_pos;
        this->y_pos2 = this->y_pos;
        break;
    }
    case 6:
        this->x_pos = this->action_x;
        this->y_pos = this->action_y;
        this->action_phase = this->action_phase + 1;
        this->phase = this->action_phase - 1;
        this->x_pos2 = this->x_pos;
        this->y_pos2 = this->y_pos;
        break;
    case 7:
        if (this->action_phase == vfx->shoot_delay && vfx->projectile != 0) {
            this->VMethod18();
        }
        if (this->action_phase == vfx->attack_delay) {
            this->VMethod21();
        }
        this->dir = this->action_dir;
        this->phase = this->action_phase;
        this->action_phase = this->action_phase + 1;
        this->x_pos2 = this->x_pos;
        this->y_pos2 = this->y_pos;
        break;
    case 8:
        if (this->action_phase == vfx->attack_delay) {
            this->VMethod20();
        }
        if (this->action_spell == 0x36) {
            if (this->action_phase == 0) {
                this->VMethod19();
            }
        } else if (this->action_phase == vfx->shoot_delay) {
            this->VMethod19();
        }
        this->phase = this->action_phase;
        this->action_phase = this->action_phase + 1;
        this->x_pos2 = this->x_pos;
        this->y_pos2 = this->y_pos;
        break;
    }

    this->last_action = this->action;
    this->action_segments = this->action_segments - 1;
    this->VMethod13();

    if (this->m_bSelected != 0) {
        main_wnd->vis_charinfo->MsgProc(0x408, 0, 0);
        main_wnd->vis_sidestatus->MsgProc(0x408, 0, 0);
    }

    return 1;
}

void CUnit::VMethod19() { // 468765
    BigStruct2* map = this->pMapObject;
    CGameObject* xd90_target = nullptr;
    if (this->action_target != 0 && map->field_0x9d0.Lookup(this->action_target, xd90_target) == 0) {
        return;
    }

    CUnit* target = (CUnit*)(xd90_target);

    CProjectile* proj = new CProjectile();
    proj->typeId = this->action_spell;
    int32_t dir_idx = (this->dir - 8) & 0xE;
    proj->action_dir = this->dir;
    proj->dir = this->dir;

    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];
    if (vfx->shoot_offset.GetSize() == 0 || this->action_spell == 0x36) {
        proj->x_pos = this->centerWorldX8 - vfx->center_x + (vfx->selection.right - vfx->selection.left) / 2;
        proj->y_pos = this->centerWorldY8 - vfx->center_y + (vfx->selection.bottom - vfx->selection.top) / 2;
    } else {
        proj->x_pos = this->centerWorldX8 + (vfx->center_x - vfx->shoot_offset[dir_idx]) * -8;
        proj->y_pos = this->centerWorldY8 + (vfx->center_y - vfx->shoot_offset[dir_idx + 1]) * -8;
    }

    proj->x_pos2 = proj->x_pos;
    proj->y_pos2 = proj->y_pos;
    proj->action_target = this->action_target;
    proj->map_player = this->map_player;

    if (this->action_target != 0) {
        proj->action_x = target->centerWorldX8;
        proj->action_y = target->centerWorldY8;
        proj->action_z = target->z_pos;
        proj->action_dir = (uint8_t)this->FUN_00461f8e(this->action_x, this->action_y);
    } else {
        proj->action_x = this->action_x;
        proj->action_y = this->action_y;
        proj->action_z = this->action_z;
    }

    proj->actionTargets.Copy(this->actionTargets);

    int32_t dx = proj->action_x - this->x_pos;
    int32_t dy = proj->action_y - this->y_pos;
    int32_t dist = (int32_t)std::sqrt((double)(dx * dx) + (double)(dy * dy));

    switch (proj->typeId) {
    case 0xA:
    case 0x12:
    case 0x28:
        proj->action_segments = dist / 200;
        break;
    case 0xC:
        proj->action_segments = dist / 0x180;
        break;
    case 0x1C:
    case 0x1E:
        proj->action_segments = 0xD;
        break;
    case 0x36:
        proj->action_segments = 0x15;
        break;
    case 0x38:
    case 0x3C:
        proj->action_segments = 1;
        break;
    }

    proj->action = 1;
    proj->action_phase = 0;
    proj->FUN_0046190d();
    map->field_0x9ec[(uint16_t)map->field_0xa24] = proj;
    map->field_0xa24 = (uint16_t)(map->field_0xa24 + 1);

    if (proj->typeId != 0x36) {
        return;
    }

    CProjectile* ghost = new CProjectile(proj);
    ghost->x_pos = ghost->action_x - vfx->center_x + (vfx->selection.right - vfx->selection.left) / 2;
    ghost->y_pos = ghost->action_y - vfx->center_y + (vfx->selection.bottom - vfx->selection.top) / 2;
    ghost->z_pos = ghost->action_z;
    ghost->FUN_0046190d();
    map->field_0x9ec[(uint16_t)map->field_0xa24] = ghost;
    map->field_0xa24 = (uint16_t)(map->field_0xa24 + 1);
}

// 469535
void CUnit::VMethod25(int32_t arg1) {
    if (0 < arg1 && arg1 < 3) {
        uint32_t now = timeGetTime();
        if (now - this->lastVoicePlaybackTick < 1500) {
            return;
        }
        this->lastVoicePlaybackTick = now;
    }

    SfxSample* sample = nullptr;
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;

    if (arg1 == 0) {
        if (g_VFX_info[this->typeId]->sound[1] != 0) {
            sample = g_SfxArray[g_VFX_info[this->typeId]->sound[1]];
        }

        map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);
        if (sample != nullptr) {
            sample->Play(vol + g_SoundSettings.sfx_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
        }
    } else {
        if ((this->unitFlags & 0x11) != 0 || this->serverId <= 0x17) {
            switch (arg1) {
            case 1:
                sample = this->FUN_0046978b()->easy;
                break;
            case 2:
                sample = this->FUN_0046978b()->hard;
                break;
            case 3:
                sample = this->FUN_0046978b()->dead;
                break;
            }
        } else if (g_VFX_info[this->typeId]->sound[arg1 + 1] != 0) {
            sample = g_SfxArray[g_VFX_info[this->typeId]->sound[arg1 + 1]];
        }

        map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);
        if (sample != nullptr) {
            sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
        }
    }
}

// 4684fb
void CUnit::VMethod18() {
    BigStruct2* map = this->pMapObject;
    CGameObject* target = nullptr;
    if (map->field_0x9d0.Lookup(this->action_target, target) == 0) {
        return;
    }

    CProjectile* proj = new CProjectile();

    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];
    proj->typeId = vfx->projectile;
    int32_t dir_idx = (this->dir - 8) & 0xE;
    proj->x_pos = this->x_pos + (vfx->center_x - vfx->shoot_offset[dir_idx]) * -8;
    proj->y_pos = this->y_pos + (vfx->center_y - vfx->shoot_offset[dir_idx + 1]) * -8;

    proj->x_pos2 = proj->x_pos;
    proj->y_pos2 = proj->y_pos;
    proj->action_target = this->action_target;
    proj->map_player = this->map_player;

    int32_t dx = this->x_pos - target->x_pos;
    int32_t dy = this->y_pos - target->y_pos;
    int32_t dist = (int32_t)std::sqrt((double)(dx * dx) + (double)(dy * dy));

    proj->action_segments = dist / 200;
    proj->action = 1;
    proj->action_phase = 0;
    proj->FUN_0046190d();
    map->field_0x9ec[(uint16_t)map->field_0xa24] = proj;
    map->field_0xa24 = (uint16_t)(map->field_0xa24 + 1);
}

// 46761e
void CUnit::VMethod9(int32_t arg1, int32_t arg2, int32_t arg3) {
    if (this->death_state > 1) {
        return;
    }

    if ((this->unitFlags & 0x80) != 0) {
        return;
    }

    BigStruct2* map = this->pMapObject;

    if (this->FUN_00462405(0x20) >= 0 && map->my_main_unit->FUN_0041ee50(this->map_player->index) == 0) {
        return;
    }

    uint16_t* land = map->field_0x80->GetLandscape();
    int32_t idx = (this->x_pos >> 8) + map->map_width * (this->y_pos >> 8);

    uint16_t mask = (land[idx] & 0xc000) | (land[idx + 1] & 0xc000) | (land[idx + map->map_width] & 0xc000) | (land[idx + map->map_width + 1] & 0xc000);
    if (mask != 0xc000) {
        return;
    }

    int32_t size;
    if (arg3 < 0) {
        arg3 = 0;
        arg1 += ((this->x_pos >> 8) - map->MapMinX()) >> 1;
        arg2 += ((this->y_pos >> 8) - map->MapMinX()) >> 1;
        size = 1;
    } else {
        arg1 += ((this->x_pos >> 8) - map->MapMinX()) << arg3;
        arg2 += ((this->y_pos >> 8) - map->MapMinX()) << arg3;
        size = 1 << arg3;
    }

    uint16_t color = g_Human_palettes[this->map_player->color]->GetPalette(8)[0xa4];
    FillRectColorSimple(arg1, arg2, arg1 + size, arg2 + size, color);
}

// 469e67
int32_t CUnit::VMethod15() {
    BigStruct2* map = this->pMapObject;

    if ((this->unitFlags & 0x80) != 0) {
        this->bIsBlocked = 1;
        if (this->m_bSelected == 0) {
            return 0;
        }
        this->m_bSelected = 0;
        this->m_bSelectionDirty = 1;
        return 1;
    }

    if (map->my_main_unit->FUN_0041ee50(this->map_player->index) != 0) {
        this->bIsBlocked = 0;
        return 0;
    }

    uint16_t* land = map->field_0x80->GetLandscape();
    int32_t idx = this->tileX + map->map_width * this->tileY;
    this->bIsBlocked = (land[idx] & 0xc000 | land[idx + 1] & 0xc000 | land[idx + map->map_width] & 0xc000 | land[idx + map->map_width + 1] & 0xc000) != 0xc000;

    if (this->bIsBlocked == 0 || this->m_bSelected == 0) {
        return 0;
    }

    this->m_bSelected = 0;
    this->m_bSelectionDirty = 1;
    return 1;
}

// 464765
void CUnit::VMethod10() {
    if (this->death_state < 2 && (this->unitFlags & 0x80) == 0) {
        this->FUN_00461c41(2, this);
    } else {
        this->FUN_00461c41(4, this);
    }

    if (g_Animation == 0 && this->FUN_0041f130() == 0 && this->m_bSelectionDirty != 0) {
        BigStruct2* map = this->pMapObject;
        UnitVFXUnfo* vfx = g_VFX_info[this->typeId];
        int32_t x = this->centerScreenX - vfx->center_x;
        int32_t y = this->centerScreenY - vfx->center_y - this->terrainHeightOffset - this->z_pos;
        CRect rect(x & 0xffffffe0, y, ((x + vfx->width) & 0xffffffe0) + 0x20, y + vfx->height);
        map->field_0xf4 |= rect;
    }

    this->m_bSelectionDirty = 0;
}

// 4698c0
void CUnit::VMethod22() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 3000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxBank* bank = this->FUN_0046978b();
    if (bank->attack_count == 0) {
        return;
    }

    SfxSample* sample = bank->attack[Random0N(bank->attack_count - 1)];
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 46999f
void CUnit::VMethod23() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 3000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxBank* bank = this->FUN_0046978b();
    if (bank->move_count == 0) {
        return;
    }

    SfxSample* sample = bank->move[Random0N(bank->move_count - 1)];
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 469a7e
void CUnit::VMethod24() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 3000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxBank* bank = this->FUN_0046978b();
    if (bank->swarm_count == 0) {
        return;
    }

    SfxSample* sample = bank->swarm[Random0N(bank->swarm_count - 1)];
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 469b5d
void CUnit::VMethod28() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 2000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxBank* bank = this->FUN_0046978b();
    if (bank->select_count == 0) {
        return;
    }

    SfxSample* sample = bank->select[Random0N(bank->select_count - 1)];
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 4693c1
void CUnit::VMethod21() {
    if (g_VFX_info[this->typeId]->sound[0] == 0) {
        return;
    }

    SfxSample* sample = g_SfxArray[g_VFX_info[this->typeId]->sound[0]];
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.sfx_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 4646c1
void CUnit::VMethod17(int16_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, MapPlayerData* arg6, int32_t arg7, int32_t arg8, int16_t arg9, int16_t arg10) {
    this->unit_id = arg1;
    this->x_pos = arg3;
    this->y_pos = arg4;
    this->x_pos2 = this->x_pos;
    this->y_pos2 = this->y_pos;
    this->z_pos = arg5;
    this->dir = arg7;
    this->phase = arg8;
    this->speed = arg9;
    this->hp = arg10;
    this->typeId = arg2;
    this->face = this->typeId;
    this->map_player = arg6;
    this->unitFlags = 0;
}

// 46f600
void CUnit::VMethod13() {
    if (this->death_state < 3) {
        CGameObject::VMethod13();
    }
}

// 46f630
void CUnit::VMethod14() {
    if (this->death_state < 3) {
        CGameObject::VMethod14();
    }
}

// 4648a1
int32_t CUnit::VMethod4() {
    return g_VFX_info[this->typeId]->tile_size;
}

// 4648c5
int32_t CUnit::VMethod5() {
    return g_VFX_info[this->typeId]->tile_size;
}

// 46f5e0
int32_t CUnit::VMethod2() {
    return 0x30;
}

// 46f5f0
int32_t CUnit::VMethod3() {
    return 0x50;
}

// 46978b
SfxBank* CUnit::FUN_0046978b() {
    switch (this->serverId) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        return &SoundBank_other[this->serverId + 5];
    case 0x15:
        return &SoundBank_other[((this->unitFlags & 4) != 0) + ((this->unitFlags & 2) != 0) * 2];
    case 0x16:
        return &SoundBank_other[4];
    case 0x17:
        return &SoundBank_other[5];
    default:
        break;
    }

    int32_t is_hero = (this->unitFlags & 4) != 0;
    if ((this->unitFlags & 2) != 0) {
        return &SoundBank_mage[is_hero];
    }
    if (this->equipmentTokens[0] != nullptr) {
        return &SoundBank_fighter[is_hero];
    }
    return &SoundBank_peasant[is_hero];
}

// 46C124
void CUnit::FUN_0046c124(CRect* rect) {
    BigStruct2* map = this->pMapObject;
    uint16_t* label_color = clrsh_DullGold;
    uint16_t* value_color = clrsh_Oxley;
    uint16_t* section_color = clrsh_CharlieBrown;

    // 0 = nothing, 7 = full stats; grows with kills on this unit type
    int32_t view_level = 0;
    if (this->serverId != 0 && this->serverId < 0xa00) {
        view_level = map->kill_stats[this->serverId] / 2;
    }
    if (this->map_player->index == 0) {
        view_level = 7;
    } else if (map->my_main_unit->FUN_0041ee50(this->map_player->index) != 0) {
        view_level = 7;
    }
    if (INT_00660f74 != 0) {
        view_level = 7;
    }

    char buf1[16];
    char buf2[16];

    if (strlen(this->str1) == 0) {
        if ((this->unitFlags & 0x11) != 0) {
            if (map->my_main_unit->FUN_0041ee20(this->map_player->index) == 0) {
                g_font2->DrawTextWithShadow(rect->right - 0x58, rect->top + 0x12, TxtFile::AllLines[0x54], 2, label_color, 1);
            } else if (this->typeId < 0x40) {
                g_font2->DrawTextWithShadow(rect->right - 0x58, rect->top + 0x12, TxtFile::AllLines[0x53], 2, label_color, 1);
            }
        }
        g_font2->DrawTextWithShadow(rect->right - 0x58, rect->top + 0x1c, txt_unitname.GetLine(this->typeId), 2, label_color, 1);
    } else {
        g_font2->DrawTextWithShadow(rect->right - 0x58, rect->top + 0x12, this->str1, 2, label_color, 1);
        g_font2->DrawTextWithShadow(rect->right - 0x58, rect->top + 0x1c, this->str2, 2, label_color, 1);
    }

    if (view_level > 4) {
        g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x2c, TxtFile::AllLines[0xf], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x36, TxtFile::AllLines[0x10], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x40, TxtFile::AllLines[0x11], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x4a, TxtFile::AllLines[0x12], 0, label_color, 1);
        sprintf(buf1, "%d", this->body);
        sprintf(buf2, "%d", this->reaction);
        g_font2->DrawTextWithShadow(rect->left + 0x50, rect->top + 0x2c, buf1, 1, value_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x50, rect->top + 0x36, buf2, 1, value_color, 1);
        sprintf(buf1, "%d", this->mind);
        sprintf(buf2, "%d", this->spirit);
        g_font2->DrawTextWithShadow(rect->left + 0x50, rect->top + 0x40, buf1, 1, value_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x50, rect->top + 0x4a, buf2, 1, value_color, 1);
    }

    if (view_level > 0) {
        g_font2->DrawTextWithShadow(rect->left + 0x73, rect->top + 0x2c, TxtFile::AllLines[0x13], 2, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x73, rect->top + 0x40, TxtFile::AllLines[0x14], 2, label_color, 1);
        sprintf(buf1, "%d/%d", this->hp, this->hp_max);
        sprintf(buf2, "%d/%d", this->mp, this->mp_max);
        g_font2->DrawTextWithShadow(rect->left + 0x73, rect->top + 0x36, buf1, 2, value_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x73, rect->top + 0x4a, buf2, 2, value_color, 1);
    }

    if (view_level > 2) {
        if ((this->unitFlags & 2) == 0) {
            g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x58, TxtFile::AllLines[0x17], 0, label_color, 1);
            g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x62, TxtFile::AllLines[0x19], 0, label_color, 1);
            uint32_t dmg_min = this->hand_dmg_min;
            uint32_t dmg_spread = this->hand_dmg_spread;
            if ((this->hand_dmg_min & 0x80) != 0 && (this->unitFlags & 0x11) == 0) {
                dmg_min = (dmg_min & 0x7f) * 0xf;
                dmg_spread = dmg_spread * 0xf;
            }
            sprintf(buf1, "%d", this->attack);
            sprintf(buf2, "%d-%d", dmg_min, dmg_min + dmg_spread);
            g_font2->DrawTextWithShadow(rect->left + 0x46, rect->top + 0x62, buf1, 1, value_color, 1);
            g_font2->DrawTextWithShadow(rect->left + 0x46, rect->top + 0x58, buf2, 1, value_color, 1);
        } else if (this->equipmentTokens[0] != nullptr && this->equipmentTokens[0]->mods_count != 0) {
            Spell spell((uint8_t)this->equipmentTokens[0]->GetCastSpellId());
            spell.sub_539541(this->equipmentTokens[0]->GetAttribute('2'));
            CString str;
            if (spell.damage_min + spell.damage_spread != 0) {
                // NB: the binary formats this string but never draws it
                str.Format("#%s: %d-%d", TxtFile::AllLines[0x76], spell.damage_min, spell.damage_min + spell.damage_spread);
                g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x58, TxtFile::AllLines[0x17], 0, label_color, 1);
                sprintf(buf2, "%d-%d", spell.damage_min, spell.damage_min + spell.damage_spread);
                g_font2->DrawTextWithShadow(rect->left + 0x46, rect->top + 0x58, buf2, 1, value_color, 1);
            }
        }
    }

    if (view_level > 3) {
        g_font2->DrawTextWithShadow(rect->left + 0x4a, rect->top + 0x58, TxtFile::AllLines[0x18], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x4a, rect->top + 0x62, TxtFile::AllLines[0x1a], 0, label_color, 1);
        sprintf(buf1, "%d", this->absorb);
        sprintf(buf2, "%d", this->defence);
        g_font2->DrawTextWithShadow(rect->left + 0x8c, rect->top + 0x58, buf1, 1, value_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x8c, rect->top + 0x62, buf2, 1, value_color, 1);
    }

    if (view_level > 1) {
        g_font2->DrawTextWithShadow(rect->left + 0x28, rect->top + 200, TxtFile::AllLines[0x15], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x28, rect->top + 0xd2, TxtFile::AllLines[0x16], 0, label_color, 1);
        sprintf(buf1, "%d.%d", this->scan_range >> 8, ((this->scan_range & 0xff) * 10) >> 8);
        sprintf(buf2, "%d", this->speed);
        g_font2->DrawTextWithShadow(rect->left + 0x6a, rect->top + 200, buf1, 1, value_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x6a, rect->top + 0xd2, buf2, 1, value_color, 1);
    }

    if (view_level > 6) {
        if ((this->unitFlags & 0x11) == 0) {
            g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x70, TxtFile::AllLines[0x1c], 0, section_color, 1);
        } else {
            g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x70, TxtFile::AllLines[0x1b], 0, section_color, 1);
        }
        for (int32_t i = 0; i < 5; i++) {
            if (this->typeId == 0x18 || this->typeId == 0x17
                || ((this->unitFlags & 1) != 0 && (this->unitFlags & 2) != 0)) {
                g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x7a + i * 10, TxtFile::AllLines[i + 0x24], 0, label_color, 1);
            } else {
                g_font2->DrawTextWithShadow(rect->left + 6, rect->top + 0x7a + i * 10, TxtFile::AllLines[i + 0x1e], 0, label_color, 1);
            }
            sprintf(buf1, "%d", this->skill_levels[i]);
            g_font2->DrawTextWithShadow(rect->left + 0x46, rect->top + 0x7a + i * 10, buf1, 1, value_color, 1);
        }
    }

    if (view_level > 5) {
        g_font2->DrawTextWithShadow(rect->left + 0x4a, rect->top + 0x70, TxtFile::AllLines[0x1c], 0, section_color, 1);
        for (int32_t i = 0; i < 5; i++) {
            g_font2->DrawTextWithShadow(rect->left + 0x4a, rect->top + 0x7a + i * 10, TxtFile::AllLines[i + 0x29], 0, label_color, 1);
            sprintf(buf1, "%d", this->magic_protect[i]);
            g_font2->DrawTextWithShadow(rect->left + 0x8c, rect->top + 0x7a + i * 10, buf1, 1, value_color, 1);
        }
    }

    if ((this->unitFlags & 1) != 0 && view_level == 7) {
        sprintf(buf1, "%d.%d", this->carrying_weight_100g / 10, this->carrying_weight_100g % 10);
        g_font2->DrawTextWithShadow(rect->left + 0x10, rect->top + 0xae, TxtFile::AllLines[0x23], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x7f, rect->top + 0xae, buf1, 1, value_color, 1);
        sprintf(buf1, "%d", this->exp_summary);
        g_font2->DrawTextWithShadow(rect->left + 0x10, rect->top + 0xba, TxtFile::AllLines[0x2e], 0, label_color, 1);
        g_font2->DrawTextWithShadow(rect->left + 0x7f, rect->top + 0xba, buf1, 1, value_color, 1);
    }

    if ((this->unitFlags & 1) == 0 && this->spells != 0 && view_level == 7) {
        g_font2->DrawTextWithShadow(rect->left + 0x4a, rect->top + 0xae, TxtFile::AllLines[0xbe], 2, label_color, 1);
    }

    if ((this->unitFlags & 0x11) == 0 && this->field_0x156 != 0 && view_level == 7) {
        g_font2->DrawTextWithShadow(rect->left + 0x4a, rect->top + 0xba, TxtFile::AllLines[0xbf], 2, label_color, 1);
    }
}

// 46D0F7
const char* CUnit::FUN_0046d0f7(int32_t x, int32_t y) {
    if (x >= 0x9f) {
        return nullptr;
    }

    BigStruct2* map = this->pMapObject;

    int32_t view_level = 0;
    if (this->serverId != 0 && this->serverId < 0xa00) {
        view_level = map->kill_stats[this->serverId] / 2;
    }
    if (this->map_player->index == 0) {
        view_level = 7;
    } else if (map->my_main_unit->FUN_0041ee50(this->map_player->index) != 0) {
        view_level = 7;
    }
    if (INT_00660f74 != 0) {
        view_level = 7;
    }

    if (view_level >= 5 && x <= 0x4f && y >= 0x2c && y <= 0x53) {
        return TxtFile::AllLines[(y - 0x2c) / 10 + 0x9b];
    }
    if (view_level >= 1 && x >= 0x50 && y >= 0x2c && y <= 0x53) {
        return TxtFile::AllLines[(y - 0x2c) / 0x14 + 0x9f];
    }
    if (view_level >= 3 && x >= 0x49 && y >= 0x58 && y <= 0x6b) {
        return TxtFile::AllLines[(y - 0x58) / 10 + 0xa1];
    }
    if (view_level >= 4 && x >= 0x4b && y >= 0x58 && y <= 0x6b) {
        return TxtFile::AllLines[(y - 0x58) / 10 + 0xa3];
    }
    if (view_level >= 2 && x >= 0x29 && x <= 0x77 && y >= 200 && y <= 0xdb) {
        return TxtFile::AllLines[(y - 0xbe) / 10 + 0xa5];
    }

    if (view_level > 6) {
        if (this->typeId == 0x18 || this->typeId == 0x17) {
            if (x < 0x50 && y > 0x79 && y < 0xac) {
                return TxtFile::AllLines[(y - 0x7a) / 10 + 0xb0];
            }
        } else if (x < 0x50 && y > 0x79 && y < 0xac) {
            return TxtFile::AllLines[(y - 0x7a) / 10 + 0xab];
        }
    }

    if (view_level > 5) {
        if ((this->unitFlags & 0x11) == 0) {
            if (x < 0x4a && y > 0x6f && y < 0x7a) {
                return TxtFile::AllLines[0xbc];
            }
        } else if (x < 0x4a && y > 0x6f && y < 0x7a) {
            return TxtFile::AllLines[0xa8];
        }
        if (x >= 0x49 && y > 0x6f && y < 0x7a) {
            return TxtFile::AllLines[0xa9];
        }
        if (x >= 0x49 && y > 0x79 && y < 0xac) {
            return TxtFile::AllLines[(y - 0x7a) / 10 + 0xb0];
        }
    }

    if ((this->unitFlags & 1) != 0 && this->map_player == map->my_main_unit) {
        if (x > 0xf && x < 0x90 && y > 0xad && y < 0xb8) {
            return TxtFile::AllLines[0xa5];
        }
        if (x > 0xf && x < 0x90 && y > 0xb9 && y < 0xc4) {
            return TxtFile::AllLines[0xaa];
        }
    }

    if ((this->unitFlags & 1) == 0 && this->spells != 0 && x > 0xf && x < 0x90 && y > 0xad && y < 0xb8) {
        CString str(TxtFile::AllLines[0xc0]);
        str += '#';

        int32_t count = 0;
        for (int32_t i = 0; i < 0x20; i++) {
            if ((this->spells & (1 << i)) != 0) {
                count++;
            }
        }
        int32_t per_column = (count - 1) / 7 + 1;

        int32_t column = 0;
        int32_t seen = 0;
        for (int32_t i = 0; i < 0x20; i++) {
            if ((this->spells & (1 << i)) != 0) {
                seen++;
                str += txt_spell.GetLine(i - 1);
                column++;
                if (seen < count) {
                    if (column == per_column) {
                        str += ",#";
                        column = 0;
                    } else {
                        str += ", ";
                    }
                }
            }
        }
        strcpy(byte_65F630, (LPCTSTR)str);
        return byte_65F630;
    }

    return nullptr;
}

// 46C0C9
int32_t CUnit::FUN_0046c0c9(TokenEntry* entry) {
    if ((entry->flg & 2) != 0 && (this->unitFlags & 2) == 0) {
        return 1;
    }
    if ((entry->flg & 4) != 0 && (this->unitFlags & 2) != 0) {
        return 1;
    }
    return 0;
}

// 4648e9
void CUnit::VMethod1(int32_t arg1) {
    BigStruct2* map = this->pMapObject;
    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    if (arg1 == 0 && map->field_0x138 == this && wnd->drag_entry != nullptr) {
        g_Cursors[CURSOR_DEFAULT]->Use();
        wnd->vis_invtype1->VMethod37(wnd->vis_invtype1->FUN_0046fb90());
        wnd->ResetItemCursor();
    }
    CGameObject::VMethod1(arg1);
}

// 469c3c
void CUnit::VMethod26() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 3000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxSample* sample = this->FUN_0046978b()->defend;
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 469cf5
void CUnit::VMethod27() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 3000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxSample* sample = this->FUN_0046978b()->retreat;
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 469dae
void CUnit::VMethod29() {
    uint32_t now = timeGetTime();
    if (now - this->lastVoicePlaybackTick < 3000) {
        return;
    }
    this->lastVoicePlaybackTick = now;

    SfxSample* sample = this->FUN_0046978b()->pickup;
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.speech_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 469499
void CUnit::VMethod20() {
    SfxSample* sample = g_SfxArray[this->action_spell + 500];
    BigStruct2* map = this->pMapObject;
    int32_t vol;
    int32_t pan;
    map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);

    if (sample != nullptr) {
        sample->Play(vol + g_SoundSettings.sfx_pos, pan, 0, (10000 - abs(vol)) / 100, 0);
    }
}

// 473b80
void StructEnter::FUN_00473b80() {
	MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
	BigStruct2* map = main_wnd->vis_map_context;
	POSITION it = map->field_0x9d0.GetStartPosition();
	this->FUN_00475010();
	while (it != NULL) {
		uint16_t key;
		CGameObject* value;
		map->field_0x9d0.GetNextAssoc(it, key, value);
		if (!value->IsKindOf(RUNTIME_CLASS(CUnit))) {
			continue;
		}
		CUnit* unit = (CUnit*)value;
		if (unit->map_player != map->my_main_unit) {
			continue;
		}
		if ((unit->unitFlags & 1) != 0 && unit->death_state == 0) {
			this->FUN_00474ff0(unit);
		} else if ((unit->unitFlags & 0x10) != 0 && unit->death_state == 0) {
			this->FUN_00475080(unit);
		}
	}
}

// 473d10
int32_t StructEnter::FUN_00473d10() {
	for (int32_t i = 0; i < this->field_0x0.GetSize(); i++) {
		CUnit* unit = this->field_0x0[i];
		if ((unit->unitFlags & 0x20) != 0) {
			return i;
		}
	}
	return -1;
}

// 475080
void StructEnter::FUN_00475080(CUnit* unit) {
	this->field_0x14.Add(unit);
	if ((unit->unitFlags & 2) != 0) {
		this->field_0x2c++;
	} else {
		this->field_0x28++;
	}
}

// 475010
void StructEnter::FUN_00475010() {
	this->field_0x0.SetSize(0);
	this->field_0x14.SetSize(0);
	this->field_0x2c = 0;
	this->field_0x28 = 0;
}

// 474ff0
void StructEnter::FUN_00474ff0(CUnit* unit) {
	this->field_0x0.Add(unit);
}

// 474f20
StructEnter::StructEnter() {
	this->field_0x28 = 0;
	this->field_0x2c = 0;
}

// 474f90
StructEnter::~StructEnter() {
	this->FUN_00475010();
}

StructEnter g_StructEnter;

// 4631f3
void CStructure::VMethod8(int32_t arg1, int32_t arg2, int32_t arg3) {
    if (arg1 != this->mapBoundsLeft || arg2 != this->mapBoundsTop) {
        return;
    }

    BigStruct2* map = this->pMapObject;
    StructureInfo* info = g_StructuresInfo[this->typeId];
    if (info->indestructible != 0) {
        return;
    }

    int32_t left = (this->tileX - map->view_x) * 0x20 + info->selection.left;
    int32_t right = (this->tileX - map->view_x) * 0x20 + info->selection.right;
    int32_t y = ((this->tileY - map->view_y) + info->tile_height - info->full_height) * 0x20 + info->selection.top - this->terrainHeightOffset;

    gfx_ball->VMethod10(left, y - 2, 0, 0, 4, 4);
    gfx_ball->VMethod10(right - 4, y - 2, 0, 0, 4, 4);

    uint16_t color_lo;
    uint16_t color_mid;
    uint16_t color_hi;
    if (this->hp < this->hp_max / 4) {
        color_lo = GetColorRGB(0xFF, 0, 0);
        color_mid = GetColorRGB(0xC0, 0, 0);
        color_hi = GetColorRGB(0x80, 0, 0);
    } else if (this->hp < this->hp_max / 2) {
        color_lo = GetColorRGB(0xFF, 0xFF, 0);
        color_mid = GetColorRGB(0xC0, 0xC0, 0);
        color_hi = GetColorRGB(0x80, 0x80, 0);
    } else {
        color_lo = GetColorRGB(0, 0xFF, 0);
        color_mid = GetColorRGB(0, 0xC0, 0);
        color_hi = GetColorRGB(0, 0x80, 0);
    }

    int32_t fill_width = this->hp * (right - left - 8) / this->hp_max;
    if (fill_width == 0 && this->hp != 0) {
        fill_width = 1;
    }

    uint16_t add_mask = g_ColorAddMask;
    if (map->show_hp != 0 && this->IsSelected() == 0) {
        color_lo = (color_lo >> 1) & add_mask;
        color_mid = (color_mid >> 1) & add_mask;
        color_hi = (color_hi >> 1) & add_mask;
    }

    this->FUN_0046246b(left, right, y, fill_width, color_lo, color_mid, color_hi);
}

// 462f58
void CStructure::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) {
    BigStruct2* map = this->pMapObject;

    int32_t sun_angle = (int32_t)(std::tan(map->field_0x80->FUN_004a7b79()) * 65536.0);

    StructureInfo* info = g_StructuresInfo[this->typeId];
    int32_t x_offset = (arg1 + map->view_x) - this->tileX;
    int32_t row_end = (arg2 + map->view_y) - this->tileY;
    int32_t row_last = row_end + info->full_height - info->tile_height;
    int32_t row_first = row_last;
    if (row_end == 0) {
        row_first = 0;
    }

    int32_t x = arg1 * 0x20;
    int32_t y = (arg2 * 0x20) - this->z_pos - this->terrainHeightOffset;

    int32_t anim_frame = 0;
    if (info->frame_count != 0) {
        anim_frame = info->frames[this->phase];
    }
    if (g_Animation == 0) {
        anim_frame = 0;
    }

    for (int32_t row = row_last; row >= row_first; row--) {
        int32_t tile_idx = row * info->tile_width + x_offset;
        int32_t frame = tile_idx;
        if (this->hp <= 0) {
            CGameBitmap* bmp = info->GetSprite();
            frame = tile_idx + bmp->GetFrameCount() - info->tile_width * info->full_height;
        } else {
            if (anim_frame != 0) {
                frame = info->tile_width * info->full_height + (anim_frame - 1) * info->field_0x38;
                char* anim_mask = info->anim_mask;
                if (anim_mask[tile_idx] == '-') {
                    frame = tile_idx;
                } else {
                    for (int32_t i = 0; i < tile_idx; i++) {
                        if (anim_mask[i] != '-') {
                            frame++;
                        }
                    }
                }
            }
        }

        int32_t shadow_offset = (int32_t)(std::tan(map->field_0x80->FUN_004a7b79()) * ((info->full_height - row) * 32.0 - info->shadow_y));

        info->GetSprite()->VMethod11(x + shadow_offset, y, frame, g_DeltaCLR.shade_delta, sun_angle, 0);
        if (g_settings.Smoothing != 0) {
            info->GetSpriteB()->VMethod11(x + shadow_offset, y, frame, g_DeltaCLR.field_10, sun_angle, 0);
        }

        y -= 0x20;
    }
}

// 462d11
void CStructure::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) {
    BigStruct2* map = this->pMapObject;
    StructureInfo* info = g_StructuresInfo[this->typeId];

    int32_t x_offset = (arg1 + map->view_x) - this->tileX;
    int32_t row_end = (arg2 + map->view_y) - this->tileY;
    int32_t row_last = row_end + info->full_height - info->tile_height;
    int32_t row_first = row_last;
    if (row_end == 0) {
        row_first = 0;
    }

    int32_t y = (arg2 * 0x20) - this->z_pos - this->terrainHeightOffset;

    int32_t anim_frame = 0;
    if (info->frame_count != 0) {
        anim_frame = info->frames[this->phase];
    }
    if (g_Animation == 0) {
        anim_frame = 0;
    }

    CGamePalette* palette = &info->GetSprite()->palette;

    for (int32_t row = row_last; row >= row_first; row--) {
        int32_t tile_idx = row * info->tile_width + x_offset;
        int32_t frame = tile_idx;
        if (this->hp <= 0 && info->indestructible == 0) {
            frame = tile_idx + info->GetSprite()->GetFrameCount() - info->tile_width * info->full_height;
        } else {
            if (anim_frame != 0) {
                frame = info->tile_width * info->full_height + (anim_frame - 1) * info->field_0x38;
                char* anim_mask = info->anim_mask;
                if (anim_mask[tile_idx] == '-') {
                    frame = tile_idx;
                } else {
                    for (int32_t i = 0; i < tile_idx; i++) {
                        if (anim_mask[i] != '-') {
                            frame++;
                        }
                    }
                }
            }
        }

        info->GetSprite()->VMethod2(arg1 * 0x20, y, frame, arg3, 0);
        if (g_settings.Smoothing != 0) {
            info->GetSpriteB()->VMethod9(arg1 * 0x20, y, frame, arg3, palette, 0);
        }

        y -= 0x20;
    }
}

// 463a58
void CStructure::VMethod12() {
    if (g_Lightning == 0) {
        return;
    }

    BigStruct2* map = this->pMapObject;
    StructureInfo* info = g_StructuresInfo[this->typeId];
    if (info->light_radius == 0 || this->hp <= 0) {
        return;
    }

    int32_t pulse_val;
    if (info->light_pulse == 0) {
        pulse_val = 0x20;
    } else {
        if (map->field_0xa88 % (info->light_pulse << 1) < info->light_pulse) {
            pulse_val = 0x20 - info->light_pulse / 2 + map->field_0xa88 % info->light_pulse;
        } else {
            pulse_val = 0x20 + info->light_pulse / 2 - map->field_0xa88 % info->light_pulse;
        }
    }

    for (int32_t x = this->mapBoundsLeft + info->light_radius - 1; x <= this->mapBoundsRight + info->light_radius; x++) {
        for (int32_t y = this->mapBoundsTop + info->light_radius - 1; y <= this->mapBoundsBottom + info->light_radius; y++) {
            if (x > -4 && x < map->view_tiles_x + 3 && y > -4 && y < map->view_tiles_y + 3) {
                map->field_0xa8[x + 3 + (y + 3) * (map->view_tiles_x + 7)] = pulse_val;
            }
        }
    }

    map->field_0x108++;

    CRect rect((this->mapBoundsLeft - 2) * 0x20, (this->mapBoundsTop - 2) * 0x20, (this->mapBoundsRight + 3) * 0x20, (this->mapBoundsBottom + 3) * 0x20);
    map->field_0xf4 |= rect;
}

// 46380c
void CStructure::VMethod9(int32_t arg1, int32_t arg2, int32_t arg3) {
    BigStruct2* map = this->pMapObject;

    uint16_t* land = map->field_0x80->GetLandscape();
    int32_t idx = (this->x_pos >> 8) + map->map_width * (this->y_pos >> 8);

    uint16_t mask = (land[idx] & 0xc000) | (land[idx + 1] & 0xc000) | (land[idx + map->map_width] & 0xc000) | (land[idx + map->map_width + 1] & 0xc000);
    if (mask != 0xc000) {
        return;
    }

    int32_t width;
    int32_t height;
    if (arg3 < 0) {
        arg1 += ((this->x_pos >> 8) - map->MapMinX()) >> 1;
        arg2 += ((this->y_pos >> 8) - map->MapMinX()) >> 1;
        width = this->VMethod4();
        height = this->VMethod5();
    } else {
        arg1 += ((this->x_pos >> 8) - map->MapMinX()) << arg3;
        arg2 += ((this->y_pos >> 8) - map->MapMinX()) << arg3;
        width = this->VMethod4() << arg3;
        height = this->VMethod5() << arg3;
    }

    uint16_t color = g_Human_palettes[this->map_player->color]->GetPalette(8)[0xa4];
    FillRectColorSimple(arg1, arg2, arg1 + width, arg2 + height, color);
}

// 462b28
int32_t CStructure::VMethod15() {
    BigStruct2* map = this->pMapObject;

    if (map->my_main_unit->FUN_0041ee50(this->map_player->index) != 0) {
        this->bIsBlocked = 0;
        return 0;
    }

    uint16_t* land = map->field_0x80->GetLandscape();
    this->bIsBlocked = 2;

    for (int32_t x = 0; x < this->VMethod4(); x++) {
        StructureInfo* info = g_StructuresInfo[this->typeId];
        for (int32_t y = this->VMethod5() - info->full_height; y < this->VMethod5(); y++) {
            int32_t idx = this->tileX + x + map->map_width * (this->tileY + y);
            uint16_t u = (land[idx] & 0xc000) | (land[idx + 1] & 0xc000) | (land[idx + map->map_width] & 0xc000) | (land[idx + map->map_width + 1] & 0xc000);
            if (u == 0x8000) {
                this->bIsBlocked = 1;
            }
            if (u == 0xc000) {
                this->bIsBlocked = 0;
                break;
            }
        }
    }

    if (this->bIsBlocked == 0 || this->m_bSelected == 0) {
        return 0;
    }

    this->m_bSelected = 0;
    this->m_bSelectionDirty = 1;
    return 1;
}

// 463a05
int32_t CStructure::VMethod11() {
    StructureInfo* info = g_StructuresInfo[this->typeId];
    if (info->frame_count != 0) {
        if (this->FUN_0041f130() == 0) {
            this->phase = (this->phase + 1) % info->frame_count;
        }
    }
    return 1;
}

// 4637ca
int32_t CStructure::VMethod4() {
    return g_StructuresInfo[this->typeId]->tile_width;
}

// 4637eb
int32_t CStructure::VMethod5() {
    return g_StructuresInfo[this->typeId]->tile_height;
}

// 462b0f
void CStructure::VMethod10() {
    this->FUN_00461c41(1, this);
}

// 4637b0
void CStructure::Dump(CDumpContext& dc) const {
    //dc << "CStructure";
}

// 46f5a0
CStructure::~CStructure() {
}

// 462acb
CStructure::CStructure() {
}

// 462aea
CStructure::CStructure(const CStructure* obj) : CGameObject(obj) {
}

// 46f550
void CBridge::VMethod17(int32_t width, int32_t height) {
    this->bridge_width = width;
    this->bridge_height = height;
}

// 46f510
int32_t CBridge::VMethod4() {
    return this->bridge_width;
}

// 46f530
int32_t CBridge::VMethod5() {
    return this->bridge_height;
}

// 46f580
CBridge::~CBridge() {
}

// 463c7f
CBridge::CBridge(int32_t width, int32_t height) {
    this->VMethod17(width, height);
}

// 463d26
void CVerticalWoodenBridge::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) {
    BigStruct2* map = this->pMapObject;
    StructureInfo* info = g_StructuresInfo[this->typeId];

    int32_t dx = (arg1 + map->view_x) - this->tileX;
    int32_t dy = (arg2 + map->view_y) - this->tileY;

    int32_t piece;
    if (dx == 0 && dy == 0) {
        piece = 0;
    } else if (dx == this->bridge_width - 1 && dy == 0) {
        piece = 2;
    } else if (dy == 0) {
        piece = 1;
    } else if (dx == 0 && dy == this->bridge_height - 1) {
        piece = 6;
    } else if (dx == this->bridge_width - 1 && dy == this->bridge_height - 1) {
        piece = 8;
    } else if (dy == this->bridge_height - 1) {
        piece = 7;
    } else if (dx == 0) {
        piece = 3;
    } else if (dx == this->bridge_width - 1) {
        piece = 5;
    } else {
        piece = 4;
    }

    int32_t x = arg1 * 0x20;
    int32_t y = (arg2 * 0x20) - this->z_pos - this->terrainHeightOffset;
    CGamePalette* palette = &info->GetSprite()->palette;

    info->GetSprite()->VMethod2(x, y, piece, arg3, 0);
    if (g_settings.Smoothing != 0) {
        info->GetSpriteB()->VMethod9(x, y, piece, arg3, palette, 0);
    }
}

// 463f01
void CVerticalWoodenBridge::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) {
}

// 4a9ef0
CVerticalWoodenBridge::~CVerticalWoodenBridge() {
}

// 4a9e90
CVerticalWoodenBridge::CVerticalWoodenBridge(int32_t width, int32_t height) : CBridge(width, height) {
}

// 463f0e
void CHorisontalWoodenBridge::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) {
    BigStruct2* map = this->pMapObject;
    StructureInfo* info = g_StructuresInfo[this->typeId];

    int32_t dx = (arg1 + map->view_x) - this->tileX;
    int32_t dy = (arg2 + map->view_y) - this->tileY;

    int32_t piece;
    if (dx == 0 && dy == 0) {
        piece = 0;
    } else if (dx == this->bridge_width - 1 && dy == 0) {
        piece = 3;
    } else if (dy == 0) {
        piece = (dx & 1) + 1;
    } else if (dx == 0 && dy == this->bridge_height - 1) {
        piece = 7;
    } else if (dx == this->bridge_width - 1 && dy == this->bridge_height - 1) {
        piece = 13;
    } else if (dy == this->bridge_height - 1) {
        if ((dx & 1) != 0) {
            piece = 12;
        } else {
            piece = (map->field_0xa88 & 3) + 8;
        }
    } else if (dx == 0) {
        piece = 4;
    } else if (dx == this->bridge_width - 1) {
        piece = 6;
    } else {
        piece = 5;
    }

    int32_t x = arg1 * 0x20;
    int32_t y = (arg2 * 0x20) - this->z_pos - this->terrainHeightOffset;
    CGamePalette* palette = &info->GetSprite()->palette;

    info->GetSprite()->VMethod2(x, y, piece, arg3, 0);
    if (g_settings.Smoothing != 0) {
        info->GetSpriteB()->VMethod9(x, y, piece, arg3, palette, 0);
    }
}

// 46410f
void CHorisontalWoodenBridge::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) {
}

// 4a9f70
CHorisontalWoodenBridge::~CHorisontalWoodenBridge() {
}

// 4a9f10
CHorisontalWoodenBridge::CHorisontalWoodenBridge(int32_t width, int32_t height) : CBridge(width, height) {
}

// 46295a
void CBackPack::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) {
    int32_t sun_angle = (int32_t)(std::tan(this->pMapObject->field_0x80->FUN_004a7b79()) * 65536.0);

    int32_t half_height = g_spr_backpack->GetHeight(0) / 2;
    int32_t shadow_offset = (int32_t)(std::tan(this->pMapObject->field_0x80->FUN_004a7b79()) * half_height);

    int32_t half_width = g_spr_backpack->GetWidth(0) / 2;
    int32_t half_height2 = g_spr_backpack->GetHeight(0) / 2;

    int32_t x = this->centerScreenX - half_width - shadow_offset;
    int32_t y = this->centerScreenY - half_height2 - this->terrainHeightOffset - 4;

    g_spr_backpack->VMethod11(x, y, this->typeId, g_DeltaCLR.shade_delta, sun_angle, 0);
    g_spr_backpackb->VMethod11(x, y, this->typeId, g_DeltaCLR.field_10, sun_angle, 0);
}

// 462887
void CBackPack::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) {
    int32_t half_width = g_spr_backpack->GetWidth(0) / 2;
    int32_t half_height = g_spr_backpack->GetHeight(0) / 2;

    int32_t x = this->centerScreenX - half_width;
    int32_t y = this->centerScreenY - half_height - this->terrainHeightOffset - 4;

    g_spr_backpack->VMethod2(x, y, this->typeId, arg3, 0);
    if (g_settings.Smoothing != 0) {
        CGamePalette* palette = &g_spr_backpack->palette;
        g_spr_backpackb->VMethod9(x, y, this->typeId, arg3, palette, 0);
    }
}

// 462a98
void CBackPack::VMethod10() {
    this->FUN_00461c41(0, this);
}

// 462ab1
void CBackPack::Dump(CDumpContext& dc) const {
    //dc << "CBackPack";
}

// 46eeb4
void CProjectile::Dump(CDumpContext& dc) const {
    //dc << "CProjectile";
}

// 46d802
CProjectile::CProjectile() {
}

// 46d85c
CProjectile::CProjectile(CProjectile* obj) : CGameObject(obj) {
    this->field_0x144.Copy(obj->field_0x144);
}

// 46f060
CProjectile::~CProjectile() {
}

// 46f5c0
int32_t CBackPack::VMethod4() {
    return 1;
}

// 46f660
int32_t CProjectile::VMethod4() {
    return 1;
}

// 46f5d0
int32_t CBackPack::VMethod5() {
    return 1;
}

// 46f670
int32_t CProjectile::VMethod5() {
    return 1;
}

// 46ef30
CBackPack::~CBackPack() {
}

// 462843
CBackPack::CBackPack() {
}

// 46d8d8
int32_t CProjectile::VMethod11() {
    BigStruct2* map = this->pMapObject;

    CRect rect((this->screenX - 0x40) & 0xffffffe0,
               this->screenY - this->terrainHeightOffset - 0x40,
               ((this->screenX + 0x40) & 0xffffffe0) + 0x20,
               this->screenY - this->terrainHeightOffset + 0x40);
    map->field_0xf4 |= rect;

    if (this->action_segments == 0) {
        if (this->typeId == 13) {
            map->force_full_redraw = 1;
        }
        return 0;
    }

    int32_t old_x = this->x_pos;
    int32_t old_y = this->y_pos;

    CGameObject* target = nullptr;

    ProjectileInfo* info = nullptr;
    if (g_ProjectileInfos.GetSize() > this->typeId && g_ProjectileInfos[this->typeId] != nullptr) {
        info = g_ProjectileInfos[this->typeId];
    }

    if (this->action == 1) {
        if (this->action_target != 0 && map->field_0x9d0.Lookup(this->action_target, target)) {
            this->action_x = target->centerWorldX8;
            this->action_y = target->centerWorldY8;
            this->action_z = target->z_pos;
            this->action_dir = (int8_t)this->FUN_00461f8e(this->action_x, this->action_y);
        }

        int32_t dx = (this->action_x - this->x_pos) / this->action_segments;
        int32_t dy = (this->action_y - this->y_pos) / this->action_segments;
        int32_t dz = (this->action_z - this->z_pos) / this->action_segments;
        this->dir = this->action_dir;
        this->action_phase += 1;

        switch (this->typeId) {
        case 13:
            if (this->action_phase == 8) {
                for (int32_t i = -1; i < 2; i++) {
                    for (int32_t j = -1; j < 2; j++) {
                        uint16_t* landscape = map->field_0x80->GetLandscape();
                        int32_t idx = this->tileX + i + (this->tileY + j) * map->map_width;
                        int32_t height = (landscape[idx] & 0x1fff) >> 6;
                        uint8_t gfx_idx = map->field_0x80->FUN_0041eec0()[idx];
                        if (height < 8 || height >= 12 || gfx_idx != 0) {
                            landscape[idx] |= 0x2000;
                        }
                    }
                }
            } else if (this->action_phase == 4) {
                for (int32_t i = -1; i < 2; i++) {
                    for (int32_t j = -1; j < 2; j++) {
                        uint16_t* landscape = map->field_0x80->GetLandscape();
                        int32_t idx = this->tileX + i + (this->tileY + j) * map->map_width;
                        if ((landscape[idx] & 0x2000) == 0) {
                            uint16_t packed = (uint16_t)((this->tileX + i) | (this->tileY + j) << 8);
                            uint8_t gfx_idx = map->field_0x80->FUN_0041eec0()[idx];
                            if (gfx_idx != 0 && g_GfxObjects[gfx_idx - 1]->dead_object != -1) {
                                map->field_0xa94.SetAt(packed, 0);
                            }
                            int32_t height = (landscape[idx] & 0x1fff) >> 6;
                            if (height > 7 && height < 12) {
                                map->field_0xa94.SetAt(packed, 0);
                            }
                        }
                    }
                }
            }
            this->phase = (this->action_phase / 2) % info->phases;
            break;
        case 16:
        case 20:
        case 24:
        case 34:
        case 44:
        case 46:
        case 48:
        case 50:
        case 62:
        case 64:
        case 66:
            if (target != nullptr) {
                this->x_pos = this->action_x;
                this->y_pos = this->action_y;
                this->phase = this->action_phase;
                target->effect_timers.Add(this->typeId << 0x10 | 0xffff);
            }
            break;
        case 23:
            this->x_pos += dx;
            this->y_pos += dy;
            this->z_pos += dz;
            this->dir = this->action_dir;
            this->phase = this->action_phase;
            if (this->action_phase == 8) {
                SfxSample* sample = g_SfxArray.GetAt(0x20b);
                int32_t vol = 0;
                int32_t pan = 0;
                map->FUN_0041b7b7(this->x_pos, this->y_pos, &vol, &pan);
                int32_t priority = (10000 - std::abs(vol)) / 100;
                if (sample != nullptr) {
                    sample->Play(vol + g_SoundSettings.sfx_pos, pan, 0, (uint8_t)priority, 0);
                }
            }
            break;
        case 28:
        case 30:
            switch (this->action_phase) {
            case 1:
            case 13:
                this->phase = 4;
                break;
            case 2:
            case 12:
                this->phase = 3;
                break;
            case 3:
            case 7:
            case 11:
                this->phase = 2;
                break;
            case 4:
            case 6:
            case 8:
            case 10:
                this->phase = 1;
                break;
            case 5:
            case 9:
                this->phase = 0;
                break;
            }
            break;
        case 54:
            this->phase = this->action_phase - 1;
            break;
        case 56:
        case 60:
            if (target != nullptr) {
                this->x_pos = this->action_x;
                this->y_pos = this->action_y;
                this->phase = this->action_phase;
                int32_t idx = target->FUN_00462405(this->typeId);
                if (idx < 0) {
                    target->effect_timers.Add(this->typeId << 0x10 | 0x20);
                } else {
                    target->effect_timers.ElementAt(idx) = this->typeId << 0x10 | 0x20;
                }
            }
            break;
        default:
            this->x_pos += dx;
            this->y_pos += dy;
            this->z_pos += dz;
            this->dir = this->action_dir;
            if (info == nullptr) {
                this->phase = 0;
            } else {
                this->phase = (this->action_phase / 2) % info->phases;
            }
            break;
        }

        if (this->typeId == 10 || this->typeId == 12) {
            if (this->field_0x144.GetSize() > 5) {
                this->field_0x144.RemoveAt(0, 1);
            }
            this->field_0x144.Add(old_x | old_y << 0x10);
        }

        this->VMethod16();
    }

    this->last_action = (int8_t)this->action;
    this->action_segments -= 1;
    this->x_pos2 = this->x_pos;
    this->y_pos2 = this->y_pos;
    return 1;
}

// 46e291
void CProjectile::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) {
    if (g_ProjectileInfos.GetSize() <= this->typeId || g_ProjectileInfos[this->typeId] == nullptr) {
        return;
    }

    BigStruct2* map = this->pMapObject;
    ProjectileInfo* info = g_ProjectileInfos[this->typeId];

    int32_t x = this->screenX - info->width / 2;
    int32_t y = (this->screenY - info->height / 2) - this->terrainHeightOffset - this->z_pos;

    int32_t flip = 0;
    int32_t rot_idx = (this->dir - 8) & 0xf;
    if (info->flip != 0 && rot_idx > 8) {
        flip = 1;
        rot_idx = 0x10 - rot_idx;
    }

    int32_t sprite_idx = this->phase + info->phases * rot_idx;
    if (info->rotation_phases == 1) {
        sprite_idx = this->phase;
    }

    switch (this->typeId) {
    case 7:
        g_DistortMap->sub_4AA29B(this->screenX, this->screenY - this->terrainHeightOffset - this->z_pos);
        break;
    case 23: {
        int32_t sub_phase;
        if (this->phase < 8) {
            sub_phase = 8;
            y -= 0x1c - this->phase * 4;
        } else {
            sub_phase = this->phase - 8;
        }
        info->FUN_0041f8f0()->VMethod2(x, y, sub_phase, arg3, 0);
        break;
    }
    case 28:
        for (int32_t i = 0; i < this->transientVisualElements.GetSize(); i++) {
            GO_11c& el = this->transientVisualElements.ElementAt(i);
            g_ProjectileInfos[28]->FUN_0041f8f0()->VMethod2(el.x - 8, el.y - 8, this->phase, 0, 0);
        }
        break;
    case 30:
        for (int32_t i = 0; i < this->transientVisualElements.GetSize(); i++) {
            GO_11c& el = this->transientVisualElements.ElementAt(i);
            g_ProjectileInfos[30]->FUN_0041f8f0()->VMethod2(el.x - 8, el.y - 8, this->phase + el.vfx_type * 5, 0, 0);
        }
        break;
    case 54:
        info->FUN_0041f8f0()->VMethod2(x, y, this->phase, arg3, 0);
        break;
    case 56:
        break;
    default:
        if (info->palette == 0) {
            info->FUN_0041f8f0()->VMethod1(x, y, sprite_idx, arg3, g_pal_projectiles, flip);
        } else {
            info->FUN_0041f8f0()->VMethod2(x, y, sprite_idx, arg3, flip);
            CSprite256* sprite2 = info->FUN_0046f760();
            if (sprite2 != nullptr) {
                sprite2->VMethod10(x, y, sprite_idx, arg3, flip);
            }
            if (this->field_0x144.GetSize() != 0) {
                int32_t smoke_idx = 0;
                if (this->typeId == 12) {
                    smoke_idx = 1;
                }
                for (int32_t i = 0; i < this->field_0x144.GetSize(); i++) {
                    uint32_t packed = this->field_0x144[i];
                    int32_t sx = ((packed & 0xffff) >> 3) - map->view_x * 0x20 - g_spr_smoke[smoke_idx]->GetWidth(0) / 2;
                    int32_t sy = (packed >> 0x13) - map->view_y * 0x20 - g_spr_smoke[smoke_idx]->GetHeight(0) / 2 - this->terrainHeightOffset - this->z_pos;
                    g_spr_smoke[smoke_idx]->VMethod2(sx, sy, i, arg3, 0);
                }
            }
        }
        break;
    }
}

// 46e805
void CProjectile::VMethod12() {
    BigStruct2* map = this->pMapObject;

    switch (this->typeId) {
    case 10:
    case 18:
    case 40:
        map->ApplyDynamicLight(this->tileX, this->tileY, 0, 0x10);
        break;
    case 12:
        map->ApplyDynamicLight(this->tileX, this->tileY, 1, 0x10);
        break;
    case 13: {
        int32_t radius = 3;
        int32_t brightness = 0;
        switch (this->phase) {
        case 0:
            radius = 1;
            brightness = 0x10;
            break;
        case 1:
            radius = 2;
            brightness = 8;
            break;
        case 4:
            brightness = 8;
            break;
        case 5:
            brightness = 0x10;
            break;
        case 6:
            brightness = 0x18;
            break;
        case 7:
            brightness = 0x20;
            break;
        case 8:
            brightness = 0x28;
            break;
        case 9:
            brightness = 0x2e;
            break;
        case 10:
            brightness = 0x2e;
            break;
        }
        map->ApplyDynamicLight(this->tileX, this->tileY, radius, brightness);
        break;
    }
    case 28:
    case 30: {
        uint8_t* lightmap = map->field_0xa8;
        for (int32_t i = 0; i < this->transientVisualElements.GetSize(); i++) {
            GO_11c& el = this->transientVisualElements.ElementAt(i);
            int32_t lx = el.x >> 5;
            int32_t ly = map->sub_4184B8(el.x, el.y);
            if (lx < 0 || ly < 0 || lx > map->view_tiles_x || ly > map->view_tiles_y + 4) {
                continue;
            }
            int32_t idx1 = lx + 3 + (ly + 3) * (map->view_tiles_x + 7);
            int32_t idx2 = idx1 + 7 + map->view_tiles_x;
            lightmap[idx1] = this->phase * 10;
            lightmap[idx1 + 1] = this->phase * 10;
            lightmap[idx2] = this->phase * 10;
            lightmap[idx2 + 1] = this->phase * 10;
            map->field_0x108 += 1;
            CRect rect((lx - 1) * 0x20, (ly - 1) * 0x20, (lx + 2) * 0x20, (ly + 2) * 0x20);
            map->field_0xf4 |= rect;
        }
        break;
    }
    default:
        if (g_Animation == 0) {
            CRect rect((this->tileX - 2) * 0x20, (this->tileY - 2) * 0x20, (this->tileX + 3) * 0x20, (this->tileY + 3) * 0x20);
            map->field_0xf4 |= rect;
        }
        break;
    }
}

// 46eb80
void CProjectile::VMethod16() {
    BigStruct2* map = this->pMapObject;

    if (this->typeId == 28) {
        TransientVisualEffectBuilder builder;
        int32_t x0 = this->screenX;
        int32_t y0 = this->screenY - this->terrainHeightOffset - this->z_pos;

        CGameObject* target = nullptr;
        if (map->field_0x9d0.Lookup(this->action_target, target) && target != nullptr) {
            int32_t x1 = target->centerScreenX;
            int32_t y1 = target->centerScreenY - target->terrainHeightOffset - target->z_pos;
            builder.FUN_004cc059(x0, y0, x1, y1, 28);
            this->transientVisualElements.Copy(*builder.GetVisualElements());
        }
    } else if (this->typeId == 30) {
        TransientVisualEffectBuilder builder;
        int32_t x0 = this->screenX;
        int32_t y0 = this->screenY - this->terrainHeightOffset - this->z_pos;

        this->transientVisualElements.RemoveAll();
        for (int32_t i = 0; i < this->actionTargets.GetSize(); i++) {
            CGameObject* target = nullptr;
            if (map->field_0x9d0.Lookup(this->actionTargets[i], target) && target != nullptr) {
                builder.FUN_004cc059(x0, y0, target->centerScreenX, target->centerScreenY - target->terrainHeightOffset - target->z_pos, i % 7);
                this->transientVisualElements.Append(*builder.GetVisualElements());
                builder.FUN_0046f390();
            }
        }
    }
}

// 46d6fb
void CAirUnit::VMethod10() {
    this->FUN_00461c41(3, this);

    if (g_Animation != 0) {
        return;
    }

    if (this->FUN_0041f130() != 0) {
        return;
    }

    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];

    int32_t x = this->centerScreenX - vfx->center_x;
    int32_t y = this->centerScreenY - vfx->center_y - this->terrainHeightOffset - this->z_pos;

    CRect rect(x & 0xffffffe0,
               y,
               ((x + vfx->width) & 0xffffffe0) + 0x20,
               y + vfx->height);
    this->pMapObject->field_0xf4 |= rect;
}

// 46d7e8
void CAirUnit::Dump(CDumpContext& dc) const {
    //dc << "CAirUnit";
}

// 46d6ad
CAirUnit::CAirUnit() {
    this->z_pos = 0x10;
}

// 46f010
CAirUnit::~CAirUnit() {
}
