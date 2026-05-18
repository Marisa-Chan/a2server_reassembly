#include "gameobj.h"
#include "main_window.h"
#include "item.h"
#include "game_app.h"

//GetRuntimeClass 461020
IMPLEMENT_DYNAMIC(CGameObject, CObject);

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
    field_0x88 = 0;
    field_0x80 = 0;
    field_0x7c = 0;
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
    field_0x18 = 0;
    spells = 0;
    field_0x20 = 0;
    field_0x114 = 1;
    active_spell = 0;

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    if (wnd)
        field_0xe8 = wnd->field_0xd0;

    field_0xe4 = 0;
    field_0xec[0] = 0;
    field_0xf8[0] = 0;
}

CGameObject::CGameObject(const CGameObject* obj)
{
    //4612f2
    unit_id = obj->unit_id;
    x_pos = obj->x_pos;
    y_pos = obj->y_pos;
    z_pos = obj->z_pos;
    field_0x14 = obj->field_0x14;
    typeId = obj->typeId;
    face = obj->face;
    x_pos2 = obj->x_pos2;
    y_pos2 = obj->y_pos2;
    field_0x34 = obj->field_0x34;
    field_0x38 = obj->field_0x38;
    field_0x3c = obj->field_0x3c;
    field_0x40 = obj->field_0x40;
    field_0x44 = obj->field_0x44;
    field_0x48 = obj->field_0x48;
    field_0x4c = obj->field_0x4c;
    field_0x50 = obj->field_0x50;
    screen_x = obj->screen_x;
    screen_y = obj->screen_y;
    field_0x54 = obj->field_0x54;
    field_0x58 = obj->field_0x58;
    field_0x6c = obj->field_0x6c;
    dir = obj->dir;
    phase = obj->phase;
    last_action = obj->last_action;
    field_0x7c = obj->field_0x7c;
    field_0x80 = obj->field_0x80;
    field_0x84 = obj->field_0x84;
    field_0x88 = obj->field_0x88;
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
    field_0x18 = obj->field_0x18;
    action_segments = obj->action_segments;
    field_0xc4 = obj->field_0xc4;
    field_0xe4 = obj->field_0xe4;
    field_0xe8 = obj->field_0xe8;
    hp = obj->hp;
    field_0x106 = obj->field_0x106;
    hp_max = obj->hp_max;
    scan_range = obj->scan_range;
    speed = obj->speed;
    carrying_weight_100g = obj->carrying_weight_100g;
    field_0x114 = obj->field_0x114;

    field_0x130.Copy(obj->field_0x130);
    field_0x11c.Copy(obj->field_0x11c);
    field_0xb0.Copy(obj->field_0xb0);

    for (int i = 0; i < obj->field_0xd0.GetSize(); i++)
        field_0xd0.Add(new GO_d0(obj->field_0xd0[i]));

    strcpy(field_0xec, obj->field_0xec);
    strcpy(field_0xf8, obj->field_0xf8);
}


void CGameObject::AssertValid() const
{ //461dfa
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    if ((mwnd->field_0x418 & 1) != 0 && 
        (x_pos < 1792 || y_pos < 1792 || 
        (field_0xe8->field_0x84 - 7) * 256 <= x_pos ||
        (field_0xe8->field_0x88 - 7) * 256 <= y_pos || 
         typeId < 0 || typeId > 256 ||
         action_x < -256 || action_y < -256 ||
         (field_0xe8->field_0x84 - 8) * 256 <= action_x || 
         (field_0xe8->field_0x88 - 8) * 256 <= action_y))
    {
        CString str;
        str.Format("Shit! Invalid GameObject #%d coordinates X:%d Y:%d player:%d picture:%d", unit_id, x_pos >> 8, y_pos >> 8, field_0x14->index, typeId);

        AfxMessageBox(str);
    }
}

void CGameObject::Dump(CDumpContext& dc) const
{ //461de0
    //dc << "CGameObject";
}

CGameObject::~CGameObject()
{ //46178a
    for (int i = 0; i < field_0xd0.GetSize(); i++)
        delete field_0xd0[i];

    field_0xd0.RemoveAll();
}


void CGameObject::FUN_0041f180(int32_t grp)
{  //41f180
    field_0x88 |= 1 << grp;
    field_0x114 = 1;
}



void CGameObject::VMethod1(int32_t arg1)
{ //46f430
    field_0x80 = arg1;
    field_0x114 = 1;
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
    if (field_0xe8->my_main_unit->FUN_0041ee50(field_0x14->index) != 0 && scan_range != 0 && (field_0x54 & 0x1f) == 0x10 && (field_0x58 & 0x1f) == 0x10 && (x_pos != field_0xc8 || y_pos != field_0xcc))
        VMethod14();
}


void CGameObject::VMethod14()
{ //462226
    if (field_0xe8->my_main_unit->FUN_0041ee50(field_0x14->index) != 0 && scan_range != 0)
    {
        field_0xe8->field_0xdc = 1;

        uint16_t* land = field_0xe8->field_0x80->GetLandscape();
        int32_t iVar6 = VMethod4();
        int32_t iVar2 = field_0x38;
        int32_t iVar7 = VMethod5();
        int32_t iVar3 = field_0x3c;

        field_0xe8->FUN_00403ca0(this);

        for (int j = 0; j < 41; j++)
        {
            for (int i = 0; i < 41; i++)
            {
                int32_t idx = iVar2 + iVar6 / 2 - 20 + j + (iVar3 + iVar7 / 2 - 20 + i) * field_0xe8->field_0x84;
                if (field_0xe8->field_0x17e4[j][i] > 0)
                {
                    land[idx] |= 0xc000;
                    idx++;

                    land[idx] |= 0xc000;
                    idx += field_0xe8->field_0x84;

                    land[idx] |= 0xc000;
                    land[idx - 1] |= 0xc000;
                }
            }
        }

        field_0xc8 = x_pos;
        field_0xcc = y_pos;
    }
}


int32_t CGameObject::VMethod15()
{ //46207a
    uint16_t* land = field_0xe8->field_0x80->GetLandscape();

    int32_t idx = field_0x38 + field_0xe8->field_0x84 * field_0x3c;
    field_0x7c = (land[idx] & 0xc000U | land[idx + 1] & 0xc000U | land[idx + field_0xe8->field_0x84] & 0xc000U | land[idx + field_0xe8->field_0x84 + 1] & 0xc000U) != 0xc000;

    if (field_0x7c == 0 || field_0x80 == 0)
        return 0;

    field_0x80 = 0;
    field_0x114 = 1;
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





GO_d0::~GO_d0()
{ //4394bf
    if (field_0xc)
        free(field_0xc);
}

GO_d0::GO_d0() = default; //438f90


GO_d0::GO_d0(int arg)
{ //439009
    field_0x4 = arg;
}


GO_d0::GO_d0(const GO_d0* obj)
{
    //439465
    operator=(*obj);
}

GO_d0::GO_d0(uint8_t** data, int arg)
{
    //439102
    field_0x18 = 0;
    field_0x14 = 0;
    field_0x1c = -1;
    field_0x20 = -1;
    field_0x9 = 0;
    field_0xa = 0;
    field_0xc = nullptr;

    uint8_t* pdata = *data;

    item_id = *(uint16_t*)pdata;
    pdata += 2;

    printf("GO_d0 %x\n", item_id);

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

        field_0xa = pkt->data_len;
        field_0x9 = pkt->field_5;
    }

    if ((flg & 0x80) == 0)
    {
        if ((flg & 0x20) == 0)
        {
            field_0x10 =  *(uint16_t*)pdata;
            pdata += 2;

            field_0xc = malloc(field_0xa);
            memcpy(field_0xc, pkt->data, field_0xa);
        }
        else
        {
            int esz = flg & 0xf;
            field_0x9 += esz + 1;
            field_0xa += (esz + 1) * 2;
            
            int local_18 = *(uint32_t*)pdata;
            pdata += 4;

            field_0xc = malloc(field_0xa);
            memcpy(field_0xc, pkt->data, field_0xa);

            uint8_t* ot = (uint8_t*)field_0xc + pkt->data_len;
            ot[0] = 0x33;
            ot[1] = 0;
            memcpy(ot + 2, pdata, esz * 2);

            pdata += esz * 2;

            memcpy((uint8_t *)field_0xc + 1, &local_18, 4);

            flg &= 0x60;
            field_0x10 = 1;
        }
    }
    else
    {
        field_0x10 = flg & 0x3f;
        field_0xc = malloc(field_0xa);
        memcpy(field_0xc, pkt->data, field_0xa);
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

void GO_d0::operator=(const GO_d0& obj)
{
    //43a3cd
    field_0x4 = obj.field_0x4;
    item_id = obj.item_id;
    flg = obj.flg;
    field_0x9 = obj.field_0x9;
    field_0x10 = obj.field_0x10;
    field_0x14 = obj.field_0x14;
    field_0x18 = obj.field_0x18;
    field_0xa = obj.field_0xa;

    if (field_0xa == 0)
        field_0xc = nullptr;
    else
    {
        field_0xc = malloc(field_0xa);
        memcpy(field_0xc, obj.field_0xc, field_0xa);
    }

    field_0x1c = obj.field_0x1c;
    field_0x20 = obj.field_0x20;
   
}