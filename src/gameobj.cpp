#include "gameobj.h"
#include "main_window.h"
#include "item.h"
#include "game_app.h"
#include "unit.h"
#include "packet.h"
#include "gfx.h"

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
        pMapObject = wnd->MapWnd;

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