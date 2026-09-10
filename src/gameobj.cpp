#include "gameobj.h"
#include "main_window.h"
#include "item.h"
#include "game_app.h"
#include "unit.h"
#include "packet.h"
#include "gfx.h"
#include "ingame.h"
#include "map_stuff.h"
#include "quest.h"
#include "quest_map.h"

#include <cmath>
#include <cstdio>
#include <cstring>

CStringArray g_CUnitMaterialSpritePaths; //660e70

//GetRuntimeClass 461020
IMPLEMENT_DYNAMIC(CGameObject, CObject);

//GetRuntimeClass 461080
IMPLEMENT_DYNAMIC(CUnit, CGameObject);

//GetRuntimeClass 461090
IMPLEMENT_DYNAMIC(CAirUnit, CUnit);

//GetRuntimeClass 461030
IMPLEMENT_DYNAMIC(CStructure, CGameObject);

CGameObject::CGameObject()
{
    //4610b0
    unit_id = 0;
    x_pos = 0;
    y_pos = 0;
    x_pos2 = 0;
    y_pos2 = 0;
    z_pos = 0;
    dir = 0;
    phase = 0;
    speed = 0;
    hp = 0;
    hp_max = 0;
    field_0x84 = -1;
    controlGroupMask = 0;
    m_bSelected = 0;
    bIsBlocked = 0;
    action_segments = 0;
    last_action = 0;
    action = 0;
    scan_range = 0;
    action_x = 0;
    action_y = 0;
    action_z = 0;
    action_dir = 0;
    action_phase = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    action_spell = 0;
    availableSpellMask = 0;
    spells = 0;
    activeSpellEffectMask = 0;
    m_bSelectionDirty = 1;
    active_spell = 0;

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    if (wnd)
        pMapObject = wnd->vis_map_context;

    shopInventoryVisibleStart = 0;
    str1[0] = 0;
    str2[0] = 0;
}

CGameObject::CGameObject(const CGameObject* obj)
{
    //4612f2
    unit_id = obj->unit_id;
    x_pos = obj->x_pos;
    y_pos = obj->y_pos;
    z_pos = obj->z_pos;
    map_player = obj->map_player;
    typeId = obj->typeId;
    face = obj->face;
    x_pos2 = obj->x_pos2;
    y_pos2 = obj->y_pos2;
    field_0x34 = obj->field_0x34;
    tileX = obj->tileX;
    tileY = obj->tileY;
    mapBoundsLeft = obj->mapBoundsLeft;
    mapBoundsTop = obj->mapBoundsTop;
    mapBoundsRight = obj->mapBoundsRight;
    mapBoundsBottom = obj->mapBoundsBottom;
    mapLayerActive = obj->mapLayerActive;
    centerScreenX = obj->centerScreenX;
    centerScreenY = obj->centerScreenY;
    screenX = obj->screenX;
    screenY = obj->screenY;
    terrainHeightOffset = obj->terrainHeightOffset;
    dir = obj->dir;
    phase = obj->phase;
    last_action = obj->last_action;
    bIsBlocked = obj->bIsBlocked;
    m_bSelected = obj->m_bSelected;
    field_0x84 = obj->field_0x84;
    controlGroupMask = obj->controlGroupMask;
    action = obj->action;
    action_x = obj->action_x;
    action_y = obj->action_y;
    action_z = obj->action_z;
    action_dir = obj->action_dir;
    action_phase = obj->action_phase;
    action_target = obj->action_target;
    field_0xa0 = obj->field_0xa0;
    field_0xa4 = obj->field_0xa4;
    action_spell = obj->action_spell;
    availableSpellMask = obj->availableSpellMask;
    action_segments = obj->action_segments;
    field_0xc4 = obj->field_0xc4;
    shopInventoryVisibleStart = obj->shopInventoryVisibleStart;
    pMapObject = obj->pMapObject;
    hp = obj->hp;
    field_0x106 = obj->field_0x106;
    hp_max = obj->hp_max;
    scan_range = obj->scan_range;
    speed = obj->speed;
    carrying_weight_100g = obj->carrying_weight_100g;
    m_bSelectionDirty = obj->m_bSelectionDirty;

    field_0x130.Copy(obj->field_0x130);
    transientVisualElements.Copy(obj->transientVisualElements);
    actionTargets.Copy(obj->actionTargets);

    for (int i = 0; i < obj->tokenEntries.GetSize(); i++)
        tokenEntries.Add(new TokenEntry(obj->tokenEntries[i]));

    strcpy(str1, obj->str1);
    strcpy(str2, obj->str2);
}


void CGameObject::AssertValid() const
{ //461dfa
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    if ((mwnd->dialogsMask & 1) != 0 && 
        (x_pos < 1792 || y_pos < 1792 || 
        (pMapObject->field_0x84 - 7) * 256 <= x_pos ||
        (pMapObject->field_0x88 - 7) * 256 <= y_pos || 
         typeId < 0 || typeId > 256 ||
         action_x < -256 || action_y < -256 ||
         (pMapObject->field_0x84 - 8) * 256 <= action_x || 
         (pMapObject->field_0x88 - 8) * 256 <= action_y))
    {
        CString str;
        str.Format("Shit! Invalid GameObject #%d coordinates X:%d Y:%d player:%d picture:%d", unit_id, x_pos >> 8, y_pos >> 8, map_player->index, typeId);

        AfxMessageBox(str);
    }
}

void CGameObject::Dump(CDumpContext& dc) const
{ //461de0
    //dc << "CGameObject";
}

CGameObject::~CGameObject()
{ //46178a
    for (int i = 0; i < tokenEntries.GetSize(); i++)
        delete tokenEntries[i];

    tokenEntries.RemoveAll();
}


void CGameObject::FUN_0041f180(int32_t grp)
{  //41f180
    controlGroupMask |= 1 << grp;
    m_bSelectionDirty = 1;
}



void CGameObject::VMethod1(int32_t arg1)
{ //46f430
    m_bSelected = arg1;
    m_bSelectionDirty = 1;
}


int32_t CGameObject::VMethod2()
{ //46f460
    return 0;
}


int32_t CGameObject::VMethod3()
{ //46f470
    return 0;
}


int32_t CGameObject::VMethod4()
{ //46f480
    return 0;
}


int32_t CGameObject::VMethod5()
{ // 46f490
    return 0;
}


void CGameObject::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3)
{ //46f4a0
}


void CGameObject::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3)
{ //46f4b0
}


void CGameObject::VMethod8(int32_t arg1, int32_t arg2, int32_t arg3)
{ //46f4c0
}


void CGameObject::VMethod9(int32_t arg1, int32_t arg2, int32_t arg3)
{ //46f4d0
}


void CGameObject::VMethod10()
{ //46f4e0
}


int32_t CGameObject::VMethod11()
{ //461f73
    VMethod13();
    return 1;
}


void CGameObject::VMethod12()
{ //46f4f0
}


void CGameObject::VMethod13()
{ //462190
    if (pMapObject->my_main_unit->FUN_0041ee50(map_player->index) != 0 && scan_range != 0 && (screenX & 0x1f) == 0x10 && (screenY & 0x1f) == 0x10 && (x_pos != occupiedLocation.x || y_pos != occupiedLocation.y))
        VMethod14();
}


void CGameObject::VMethod14()
{ //462226
    if (pMapObject->my_main_unit->FUN_0041ee50(map_player->index) != 0 && scan_range != 0)
    {
        pMapObject->field_0xdc = 1;

        uint16_t* land = pMapObject->field_0x80->GetLandscape();
        int32_t iVar6 = VMethod4();
        int32_t iVar2 = tileX;
        int32_t iVar7 = VMethod5();
        int32_t iVar3 = tileY;

        pMapObject->FUN_00403ca0(this);

        for (int j = 0; j < 41; j++)
        {
            for (int i = 0; i < 41; i++)
            {
                int32_t idx = iVar2 + iVar6 / 2 - 20 + j + (iVar3 + iVar7 / 2 - 20 + i) * pMapObject->field_0x84;
                if (pMapObject->field_0x17e4[j][i] > 0)
                {
                    land[idx] |= 0xc000;
                    idx++;

                    land[idx] |= 0xc000;
                    idx += pMapObject->field_0x84;

                    land[idx] |= 0xc000;
                    land[idx - 1] |= 0xc000;
                }
            }
        }

        occupiedLocation.x = x_pos;
        occupiedLocation.y = y_pos;
    }
}


int32_t CGameObject::VMethod15()
{ //46207a
    uint16_t* land = pMapObject->field_0x80->GetLandscape();

    int32_t idx = tileX + pMapObject->field_0x84 * tileY;
    bIsBlocked = (land[idx] & 0xc000U | land[idx + 1] & 0xc000U | land[idx + pMapObject->field_0x84] & 0xc000U | land[idx + pMapObject->field_0x84 + 1] & 0xc000U) != 0xc000;

    if (bIsBlocked == 0 || m_bSelected == 0)
        return 0;

    m_bSelected = 0;
    m_bSelectionDirty = 1;
    return 1;
}

void CGameObject::VMethod16()
{ // 46f500
}

void CGameObject::SetVals(uint16_t uni_id, int type_id, int32_t xpos, int32_t ypos, int32_t unk1, int32_t unk2, int32_t _phase, int32_t _speed, int32_t _hp)
{ //46187d
    unit_id = uni_id;
    x_pos = xpos;
    x_pos2 = xpos;
    y_pos = ypos;
    y_pos2 = ypos;
    z_pos = unk1;
    dir = unk2;
    phase = _phase;
    speed = _speed;
    hp_max = _hp;
    hp = _hp;
    typeId = type_id;
}





TokenEntry::~TokenEntry()
{ //4394bf
    if (mods)
        free(mods);
}

TokenEntry::TokenEntry() = default; //438f90


TokenEntry::TokenEntry(int arg)
{ //439009
    field_0x4 = arg;
}


TokenEntry::TokenEntry(const TokenEntry* obj)
{
    //439465
    operator=(*obj);
}

TokenEntry::TokenEntry(uint8_t** data, int arg)
{
    //439102
    field_0x18 = 0;
    field_0x14 = 0;
    field_0x1c = -1;
    field_0x20 = -1;
    mods_count = 0;
    mods_size = 0;
    mods = nullptr;

    uint8_t* pdata = *data;

    item_id = *(uint16_t*)pdata;
    pdata += 2;

    flg = *pdata;
    pdata++;

    ItemNamePktEntry* pkt = nullptr;
    if (item_id < 0xfff0)
    {
        if (unk_660D28.Lookup(item_id, *(void**)&pkt) == 0)
        {
            *data = pdata;
            return;
        }

        mods_size = pkt->data_len;
        mods_count = pkt->field_5;
    }

    if ((flg & 0x80) == 0)
    {
        if ((flg & 0x20) == 0)
        {
            field_0x10 =  *(uint16_t*)pdata;
            pdata += 2;

            mods = malloc(mods_size);
            memcpy(mods, pkt->data, mods_size);
        }
        else
        {
            int esz = flg & 0xf;
            mods_count += esz + 1;
            mods_size += (esz + 1) * 2;
            
            int local_18 = *(uint32_t*)pdata;
            pdata += 4;

            mods = malloc(mods_size);
            memcpy(mods, pkt->data, mods_size);

            uint8_t* ot = (uint8_t*)mods + pkt->data_len;
            ot[0] = 0x33;
            ot[1] = 0;
            memcpy(ot + 2, pdata, esz * 2);

            pdata += esz * 2;

            memcpy((uint8_t *)mods + 1, &local_18, 4);

            flg &= 0x60;
            field_0x10 = 1;
        }
    }
    else
    {
        field_0x10 = flg & 0x3f;
        mods = malloc(mods_size);
        memcpy(mods, pkt->data, mods_size);
        flg &= 0x40;
    }

    if (pkt)
        flg |= pkt->flags;

    if (arg != 0)
    {
        int val = *pdata;
        pdata++;

        if (val == 0)
            field_0x18 = 2;
        else
            field_0x18 = val + 4;
    }

    *data = pdata;
}

void TokenEntry::operator=(const TokenEntry& obj)
{
    //43a3cd
    field_0x4 = obj.field_0x4;
    item_id = obj.item_id;
    flg = obj.flg;
    mods_count = obj.mods_count;
    field_0x10 = obj.field_0x10;
    field_0x14 = obj.field_0x14;
    field_0x18 = obj.field_0x18;
    mods_size = obj.mods_size;

    if (mods_size == 0)
        mods = nullptr;
    else
    {
        mods = malloc(mods_size);
        memcpy(mods, obj.mods, mods_size);
    }

    field_0x1c = obj.field_0x1c;
    field_0x20 = obj.field_0x20;
   
}



CUnit::CUnit()
{ //46411c
    mp_max = 0;
    mp = 0;
    lastVoicePlaybackTick = 0;
    equipmentTokens.fill(nullptr);
    sprite = nullptr;
    sprite_b = nullptr;
    heroSpritePictureName[0] = 0;
    y_pos2 = 0;
    x_pos2 = 0;
    unitFlags = 0;
    field_0x180[4] = 0;
    heroSpriteArmorMaterial = -1;
    field_0x161 = 0;
    field_0x162 = 0;
    field_0x164 = 0;
    serverId = 0;
    questFlags = -1;
    field_0x156 = 0;
}

CUnit::~CUnit()
{ //46459d
    for (TokenEntry* eq : equipmentTokens)
    {
        if (eq)
            delete eq;
    }

    if (sprite)
        delete sprite;

    if (sprite_b)
        delete sprite_b;
}

void CUnit::VMethod30(const char* str, CGameBitmap* bmp1, CGameBitmap* bmp2)
{ // 469ffc
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
    CSprite256* extra_sprites[10] = { nullptr };

    for (int i = 0; i < 12; ++i) {
        if (equipmentTokens[i] != nullptr) {
            CString base_name = sprite_prefix;
            CString extra_name = sprite_prefix;
            CString item_name = equipmentTokens[i]->FUN_004394f3();
            CString item_name2 = equipmentTokens[i]->FUN_004394f3();

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


void CUnit::CopyFromUnit(const Unit& uni)
{ //46b0d7
    typeId = uni.typeId;
    ApplyFace(uni.face);
    body = uni.body;
    mind = uni.mind;
    reaction = uni.reaction;
    spirit = uni.spirit;
    hp = uni.hp;
    hp_max = uni.hp_max;
    mp = uni.mp;
    mp_max = uni.mp_max;
    attack = uni.hit_values.attack;
    defence = uni.protections.defense;
    absorb = uni.protections.absorption;
    hand_dmg_min = uni.hit_values.hand_damage_min;
    hand_dmg_spread = uni.hit_values.hand_damage_spread;
    speed = uni.speed;
    scan_range = uni.scan_range;
    serverId = uni.server_id;
    carrying_weight_100g = uni.carrying_weight_100g;

    if (((Unit&)uni).VMethod8() == 0) //FIXME when this method become const
    {
        skill_levels[0] = uni.protections.weapon_protections[1];
        skill_levels[1] = uni.protections.weapon_protections[2];
        skill_levels[2] = uni.protections.weapon_protections[3];
        skill_levels[3] = uni.protections.weapon_protections[4];
        skill_levels[4] = uni.protections.weapon_protections[5];
    }
    else
    {
        Humanoid& hum = (Humanoid&)uni;
        skill_levels[0] = hum.hit_values.skill_levels[1];
        skill_levels[1] = hum.hit_values.skill_levels[2];
        skill_levels[2] = hum.hit_values.skill_levels[3];
        skill_levels[3] = hum.hit_values.skill_levels[4];
        skill_levels[4] = hum.hit_values.skill_levels[5];

        experience_per_sphere[0] = hum.experience_per_sphere[0];
        experience_per_sphere[1] = hum.experience_per_sphere[1];
        experience_per_sphere[2] = hum.experience_per_sphere[2];
        experience_per_sphere[3] = hum.experience_per_sphere[3];
        experience_per_sphere[4] = hum.experience_per_sphere[4];

        exp_summary = experience_per_sphere[0] + experience_per_sphere[1] + experience_per_sphere[2] + experience_per_sphere[3] + experience_per_sphere[4];

        PacketUnitStateVec* local_20 = &PacketUnitStateVec::Inst;
        local_20->entry_count = 0;
        local_20->field_0xf = 0;
        local_20->data_size = 0;

        if (uni.weapon)
            uni.weapon->StoreToPacket(local_20, 0);
        else
        {
            Item itm;
            itm.StoreToPacket(local_20, 0);
        }

        if (uni.shield)
            uni.shield->StoreToPacket(local_20, 0);
        else
        {
            Item itm;
            itm.StoreToPacket(local_20, 0);
        }

        for (int i = 3; i < 13; i++)
        {
            if (hum.equipment[i])
                hum.equipment[i]->StoreToPacket(local_20, 0);
            else
            {
                Item itm;
                itm.StoreToPacket(local_20, 0);
            }
        }

        uint8_t* pdata = local_20->data;
        for (int i = 0; i < 12; i++)
        {
            if (equipmentTokens[i])
                delete equipmentTokens[i];

            TokenEntry* te = new TokenEntry(&pdata, 0);
            if (te->item_id == 0)
            {
                delete te;
                equipmentTokens[i] = nullptr;
            }
            else
            {
                te->field_0x18 = 1;
                equipmentTokens[i] = te;
            }
        }
    }

    magic_protect[0] = uni.protections.magic_protections[1];
    magic_protect[1] = uni.protections.magic_protections[2];
    magic_protect[2] = uni.protections.magic_protections[3];
    magic_protect[3] = uni.protections.magic_protections[4];
    magic_protect[4] = uni.protections.magic_protections[5];

    if (typeId == 0x49 || typeId == 0x68 || typeId == 0x46)
        field_0x156 = 2;
    else
        field_0x156 = 0;

    ReloadSprite();
}

void CUnit::ApplyFace(int32_t _face)
{ //46b7d2

    face = _face;

    if (typeId > 0x1f && typeId < 0x40)
    {
        unitFlags &= 0x80;
        unitFlags |= 1 | 8;

        typeId -= 0x21;

        if ((typeId & 1) != 0)
            unitFlags |= 4;

        if ((typeId & 2) != 0)
            unitFlags |= 2;

        typeId = 1;
    }
    else if (typeId < 0x1a)
    {
        unitFlags &= 0x80;
        unitFlags |= 0x10 | 8;

        if ((face & 0x80) != 0)
            unitFlags |= 4;

        face &= ~0x80;

        if (typeId > 0x16 && typeId < 0x19)
            unitFlags |= 2;
    }
}

void CUnit::ReloadSprite()
{ // 46b91c
    if ((unitFlags & 1) != 0)
    {
        CString sname;

        if (action == 6)
        {
            sname = "unarmed";
            if ((unitFlags & 2) != 0 /* && strcmp(local_c8, "unarmed") == 0 */)
                sname = "mage_st";
        }
        else
        {
            int32_t hero_id = 0;

            if (equipmentTokens[0] != nullptr)
                hero_id = equipmentTokens[0]->GetId() - 1;

            sname = txt_heropicture.GetLine(hero_id);

            if (equipmentTokens[1] != NULL)
                sname += "_";

            if ((unitFlags & 2) != 0 && sname == "unarmed")
                sname = "mage";
        }

        bool do_reload = sname != heroSpritePictureName;

        if ((unitFlags & 2) == 0)
        {
            if (equipmentTokens[7] == nullptr)
            {
                if (heroSpriteArmorMaterial != -1)
                    do_reload = true;
            }
            else if (heroSpriteArmorMaterial == -1)
                do_reload = true;
        }

        if (do_reload)
        {
            if (sprite)
                delete sprite;

            if (sprite_b)
                delete sprite_b;

            CString sprite_name = "graphics\\units\\";

            if (unitFlags & 2)
                sprite_name += "heroes";
            else if (equipmentTokens[7] == nullptr)
            {
                heroSpriteArmorMaterial = -1;
                sprite_name += "heroes_l";
            }
            else
            {
                sprite_name += g_CUnitMaterialSpritePaths[equipmentTokens[7]->GetMaterial()];
                heroSpriteArmorMaterial = equipmentTokens[7]->GetMaterial();
            }

            sprite_name += "\\";
            sprite_name += sname;

            CString sprite_name_b = sprite_name;

            sprite_name += "\\sprites.256";
            sprite_name_b += "\\spritesb.256";

            sprite = new CSprite256(sprite_name);
            sprite_b = new CSprite256(sprite_name_b);

            strcpy(heroSpritePictureName, sname);

            if (strcmp(heroSpritePictureName, "unarmed") == 0)
                typeId = 1;
            else if (strcmp(heroSpritePictureName, "unarmed_") == 0)
                typeId = 2;
            else if (strcmp(heroSpritePictureName, "swordsman") == 0)
                typeId = 3;
            else if (strcmp(heroSpritePictureName, "swordsman_") == 0)
                typeId = 4;
            else if (strcmp(heroSpritePictureName, "swordsman2h") == 0)
                typeId = 5;
            else if (strcmp(heroSpritePictureName, "axeman") == 0)
                typeId = 7;
            else if (strcmp(heroSpritePictureName, "axeman_") == 0)
                typeId = 8;
            else if (strcmp(heroSpritePictureName, "axeman2h") == 0)
                typeId = 9;
            else if (strcmp(heroSpritePictureName, "clubman") == 0)
                typeId = 10;
            else if (strcmp(heroSpritePictureName, "clubman_") == 0)
                typeId = 11;
            else if (strcmp(heroSpritePictureName, "pikeman") == 0)
                typeId = 12;
            else if (strcmp(heroSpritePictureName, "pikeman_") == 0)
                typeId = 13;
            else if (strcmp(heroSpritePictureName, "archer") == 0)
                typeId = 14;
            else if (strcmp(heroSpritePictureName, "bowman") == 0)
                typeId = 14;
            else if (strcmp(heroSpritePictureName, "xbowman") == 0)
                typeId = 15;
            else if (strcmp(heroSpritePictureName, "mage") == 0)
                typeId = 23;
            else if (strcmp(heroSpritePictureName, "mage_st") == 0)
                typeId = 24;
        }
    }
}

void CGameObject::FUN_0046246b(int32_t left, int32_t right, int32_t y, int32_t fill_width, uint16_t color_lo, uint16_t color_mid, uint16_t color_hi)
{ // 46246B
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

void CGameObject::FUN_004627bb(int32_t x, int32_t y)
{ // 4627BB
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

void CUnit::VMethod8(int32_t arg1, int32_t arg2, int32_t arg3)
{ // 466a78
    if (this->field_0x180[4] >= 5) {
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

void CUnit::VMethod6(int32_t arg1, int32_t arg2, int32_t arg3)
{ // 464974
    if (this->field_0x180[4] >= 5) {
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
        if (elem->field_0x4 > 0) {
            ProjectileInfo* proj = g_ProjectileInfos[elem->field_0x6];
            int32_t x = this->centerScreenX + elem->field_0x0 - proj->width / 2;
            int32_t y = this->centerScreenY - this->terrainHeightOffset - this->z_pos + elem->field_0x2 - proj->height / 2 - elem->field_0x4;
            proj->FUN_0041f8f0()->VMethod2(x, y, elem->field_0x7, 0, 0);
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
        } else if (this->field_0x180[4] == 1) {
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
        } else if (this->field_0x180[4] <= 1 || g_VFX_info[vfx->dying]->bone_phases == 0) {
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
                frame = (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 5 + dir8 * dying_vfx->bone_phases + 9 + this->field_0x180[4] - 2;
            } else {
                frame = dir8 * dying_vfx->bone_phases + (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 8 + 0x10 + this->field_0x180[4] - 2;
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

    if (this->FUN_00462405(0x2C) >= 0 && this->field_0x180[4] <= 2) {
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

    if (this->FUN_00462405(0x2C) >= 0 && this->field_0x180[4] <= 2) {
        if (palette_id == 0) {
            palette = g_Human_palettes[0x10];
        } else {
            palette = &tmp_palette;
            tmp_palette.SetPalette((RGBQUAD*)g_VFX_info[vfx_idx]->FUN_0046f6e0(0), 0x10, 5, 0);
        }
    }

    if (map->field_0x994 == this && main_wnd->field_0x408 == NULL) {
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
        if (elem->field_0x4 <= 0) {
            ProjectileInfo* proj = g_ProjectileInfos[elem->field_0x6];
            int32_t x = this->centerScreenX + elem->field_0x0 - proj->width / 2;
            int32_t y = this->centerScreenY - this->terrainHeightOffset - this->z_pos + elem->field_0x2 - proj->height / 2 - elem->field_0x4;
            proj->FUN_0041f8f0()->VMethod2(x, y, elem->field_0x7, 0, 0);
        }
    }
}

void CUnit::VMethod7(int32_t arg1, int32_t arg2, int32_t arg3)
{ // 465b5a
    if (this->field_0x180[4] >= 5) {
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
        } else if (this->field_0x180[4] == 1) {
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
        } else if (this->field_0x180[4] <= 1 || g_VFX_info[vfx->dying]->bone_phases == 0) {
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
                frame = (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 5 + dir8 * dying_vfx->bone_phases + 7 + this->field_0x180[4];
            } else {
                frame = dir8 * dying_vfx->bone_phases + (base_phases + dying_vfx->attack_phases + dying_vfx->dying_phases) * 8 + 14 + this->field_0x180[4];
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

    if (this->FUN_00462405(0x2C) >= 0 && this->field_0x180[4] <= 2) {
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
            this->sprite->VMethod11(x, y, frame, g_DeltaCLR.field_c, sun_angle, flip);
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
                gfx_file->FUN_0046f700()->VMethod3(x + shadow_offset + sun_angle / 2000, y, frame, g_DeltaCLR.field_c, flip);
            } else {
                gfx_file->FUN_0046f700()->VMethod11(x, y, frame, g_DeltaCLR.field_c, sun_angle, flip);
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

int32_t CUnit::VMethod11()
{ // 46785f
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    UnitVFXUnfo* vfx = g_VFX_info[this->typeId];

    if (this->field_0x180[4] >= 5) {
        return 1;
    }

    this->VMethod16();

    if (this->action_segments == 0) {
        if (this->last_action != 0 || this->field_0x130.GetSize() != 0 || g_VFX_info[this->typeId]->idle_phases > 0) {
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
        this->field_0xa0 += std::abs(dx);
        this->action_y -= dy;
        this->field_0xa4 += std::abs(dy);
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
            switch ((this->field_0xa4 / 26) & 7) {
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
            switch ((this->field_0xa0 / 32) & 7) {
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
            switch ((this->field_0xa0 / 25) & 7) {
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
        int32_t turn = this->action_dir * 16 - this->field_0xc4;
        if (turn < -0x7F) {
            turn += 0x100;
        }
        if (turn > 0x80) {
            turn -= 0x100;
        }
        this->field_0xc4 += turn / this->action_segments;
        if (this->field_0xc4 < 0) {
            this->field_0xc4 += 0x100;
        }
        this->dir = this->field_0xc4 >> 4;
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
