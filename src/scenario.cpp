#include "game_app.h"

ScenarioState g_scenarioState; // 10036028
ScenarioLocation* g_currentScenarioLocation; //10037560
CList<ScenarioLocation*> g_allLocations; //10037028
CList<ScenarioLocation*> g_availableLocations; //10035f90
AssortGenParams g_ShopsAssortment[4][4]; //10037420

ScenarioLocation g_scenarioLocationTown1_1(2, 1, 215, 234, 0, 0); //10035f60
ScenarioLocation g_scenarioLocationTown1_2(2, 2, 370, 305, 0, 0); //10035f78
ScenarioLocation g_scenarioLocationTown3(2, 3, 194, 225, 0, 0); //10037568
ScenarioLocation g_scenarioLocationTown10(1, 10, 380, 364, 0, 0); //10037580
ScenarioLocation g_scenarioLocationTown20(1, 20, 210, 260, 0, 0); //10037598
ScenarioLocation g_scenarioLocationTown30(1, 30, 511, 301, 0, 0); //100375b0
ScenarioLocation g_scenarioLocationMission73(1, 73, 343, 347, 0, 0); //10037048
ScenarioLocation g_scenarioLocationMission74(1, 74, 595, 313, 0, 0); //10037060
ScenarioLocation g_scenarioLocationMission80(1, 80, 554, 299, 0, 0); //10037078
ScenarioLocation g_scenarioLocationMission71(1, 71, 337, 306, 0, 0); //10037090
ScenarioLocation g_scenarioLocationMission72(1, 72, 530, 255, 0, 0); //100370a8
ScenarioLocation g_scenarioLocationMission83(1, 83, 521, 372, 0, 0); //100370c0
ScenarioLocation g_scenarioLocationMission77(1, 77, 406, 117, 0, 0); //100370d8
ScenarioLocation g_scenarioLocationMission84(1, 84, 552, 377, 0, 0); //100370f0
ScenarioLocation g_scenarioLocationMission90(1, 90, 283, 244, 0, 0); //10037108
ScenarioLocation g_scenarioLocationMission81(1, 81, 510, 182, 0, 0); //10037120
ScenarioLocation g_scenarioLocationMission75(1, 75, 356, 359, 0, 0); //10037138
ScenarioLocation g_scenarioLocationMission82(1, 82, 424, 186, 0, 0); //10037150
ScenarioLocation g_scenarioLocationMission76(1, 76, 111, 227, 0, 0); //10037168
ScenarioLocation g_scenarioLocationMission93(1, 93, 306, 148, 0, 0); //10037180
ScenarioLocation g_scenarioLocationMission87(1, 87, 509, 161, 0, 0); //10037198
ScenarioLocation g_scenarioLocationMission94(1, 94, 306, 148, 0, 0); //100371b0
ScenarioLocation g_scenarioLocationMission91(1, 91, 477, 269, 0, 0); //100371c8
ScenarioLocation g_scenarioLocationMission85(1, 85, 393, 360, 0, 0); //100371e0
ScenarioLocation g_scenarioLocationMission92(1, 92, 93, 269, 0, 0); //100371f8
ScenarioLocation g_scenarioLocationMission86(1, 86, 67, 213, 0, 0); //10037210
ScenarioLocation g_scenarioLocationMission95(1, 95, 306, 148, 0, 0); //10037228
ScenarioLocation g_scenarioLocationMission96(1, 96, 306, 148, 0, 0); //10037240
ScenarioLocation g_scenarioLocationMission10(1, 10, 302, 158, 0, 0); //10037258
ScenarioLocation g_scenarioLocationMission20(1, 20, 380, 364, 0, 0); //10037270
ScenarioLocation g_scenarioLocationMission30(1, 30, 331, 395, 0, 0); //10037288
ScenarioLocation g_scenarioLocationMission21(1, 21, 423, 367, 0, 0); //100372a0
ScenarioLocation g_scenarioLocationMission40(1, 40, 339, 226, 0, 0); //100372b8
ScenarioLocation g_scenarioLocationMission31(1, 31, 443, 225, 0, 0); //100372d0
ScenarioLocation g_scenarioLocationMission32(1, 32, 457, 202, 0, 0); //100372e8
ScenarioLocation g_scenarioLocationMission43(1, 43, 454, 358, 0, 0); //10037300
ScenarioLocation g_scenarioLocationMission50(1, 50, 210, 260, 0, 0); //10037318
ScenarioLocation g_scenarioLocationMission41(1, 41, 403, 228, 0, 0); //10037330
ScenarioLocation g_scenarioLocationMission42(1, 42, 531, 207, 0, 0); //10037348
ScenarioLocation g_scenarioLocationMission53(1, 53, 292, 191, 0, 0); //10037360
ScenarioLocation g_scenarioLocationMission60(1, 60, 511, 301, 0, 0); //10037378
ScenarioLocation g_scenarioLocationMission51(1, 51, 450, 240, 0, 0); //10037390
ScenarioLocation g_scenarioLocationMission52(1, 52, 132, 303, 0, 0); //100373a8
ScenarioLocation g_scenarioLocationMission63(1, 63, 385, 221, 0, 0); //100373c0
ScenarioLocation g_scenarioLocationMission70(1, 70, 86, 193, 0, 0); //100373d8
ScenarioLocation g_scenarioLocationMission61(1, 61, 475, 312, 0, 0); //100373f0
ScenarioLocation g_scenarioLocationMission62(1, 62, 253, 169, 0, 0); //10037408
ScenarioLocation g_scenarioLocationMission100(1, 100, 388, 140, 0, 0); //10035fe0
ScenarioLocation g_scenarioLocationMission101(1, 101, 273, 300, 0, 0); //10035fc8
ScenarioLocation g_scenarioLocationMission102(1, 102, 351, 190, 0, 0); //10036010
ScenarioLocation g_scenarioLocationMission103(1, 103, 575, 362, 0, 0); //10035ff8
ScenarioLocation g_scenarioLocationMission110(1, 110, 448, 140, 0, 0); //10035fb0



void InitAllScenatioLocations()
{ //10001a0f
    g_allLocations.RemoveAll();
    g_allLocations.AddTail(&g_scenarioLocationTown1_1);
    g_allLocations.AddTail(&g_scenarioLocationTown1_2);
    g_allLocations.AddTail(&g_scenarioLocationTown3);
    g_allLocations.AddTail(&g_scenarioLocationMission10);
    g_allLocations.AddTail(&g_scenarioLocationMission20);
    g_allLocations.AddTail(&g_scenarioLocationMission21);
    g_allLocations.AddTail(&g_scenarioLocationMission30);
    g_allLocations.AddTail(&g_scenarioLocationMission31);
    g_allLocations.AddTail(&g_scenarioLocationMission32);
    g_allLocations.AddTail(&g_scenarioLocationMission40);
    g_allLocations.AddTail(&g_scenarioLocationMission41);
    g_allLocations.AddTail(&g_scenarioLocationMission42);
    g_allLocations.AddTail(&g_scenarioLocationMission43);
    g_allLocations.AddTail(&g_scenarioLocationMission50);
    g_allLocations.AddTail(&g_scenarioLocationMission51);
    g_allLocations.AddTail(&g_scenarioLocationMission52);
    g_allLocations.AddTail(&g_scenarioLocationMission53);
    g_allLocations.AddTail(&g_scenarioLocationMission60);
    g_allLocations.AddTail(&g_scenarioLocationMission61);
    g_allLocations.AddTail(&g_scenarioLocationMission62);
    g_allLocations.AddTail(&g_scenarioLocationMission63);
    g_allLocations.AddTail(&g_scenarioLocationMission70);
    g_allLocations.AddTail(&g_scenarioLocationMission71);
    g_allLocations.AddTail(&g_scenarioLocationMission72);
    g_allLocations.AddTail(&g_scenarioLocationMission73);
    g_allLocations.AddTail(&g_scenarioLocationMission74);
    g_allLocations.AddTail(&g_scenarioLocationMission75);
    g_allLocations.AddTail(&g_scenarioLocationMission76);
    g_allLocations.AddTail(&g_scenarioLocationMission77);
    g_allLocations.AddTail(&g_scenarioLocationMission80);
    g_allLocations.AddTail(&g_scenarioLocationMission81);
    g_allLocations.AddTail(&g_scenarioLocationMission82);
    g_allLocations.AddTail(&g_scenarioLocationMission83);
    g_allLocations.AddTail(&g_scenarioLocationMission84);
    g_allLocations.AddTail(&g_scenarioLocationMission85);
    g_allLocations.AddTail(&g_scenarioLocationMission86);
    g_allLocations.AddTail(&g_scenarioLocationMission87);
    g_allLocations.AddTail(&g_scenarioLocationMission90);
    g_allLocations.AddTail(&g_scenarioLocationMission91);
    g_allLocations.AddTail(&g_scenarioLocationMission92);
    g_allLocations.AddTail(&g_scenarioLocationMission93);
    g_allLocations.AddTail(&g_scenarioLocationMission94);
    g_allLocations.AddTail(&g_scenarioLocationMission95);
    g_allLocations.AddTail(&g_scenarioLocationMission96);
    g_allLocations.AddTail(&g_scenarioLocationMission100);
    g_allLocations.AddTail(&g_scenarioLocationMission101);
    g_allLocations.AddTail(&g_scenarioLocationMission102);
    g_allLocations.AddTail(&g_scenarioLocationMission103);
    g_allLocations.AddTail(&g_scenarioLocationMission110);
}


void __stdcall ScenarioEnterInn(int32_t *ids, int32_t* ids_num)
{ // 1000286b
    *ids_num = 0;

    switch (g_scenarioState.scenarioChapter)
    {
    case 10:
        ids[*ids_num] = 0x900cf;
        *ids_num += 1;
        ids[*ids_num] = 0x8083c;
        *ids_num += 1;
        ids[*ids_num] = 0x300a0205;
        *ids_num += 1;
        break;

    case 30:
        ids[*ids_num] = 0x301e0016;
        *ids_num += 1;

        if (g_scenarioState.completedLocationFlags[31] == 0)
        {
            ids[*ids_num] = 0x301f083c;
            *ids_num += 1;
        }

        ids[*ids_num] = 0x27083e;
        *ids_num += 1;
        break;

    case 40:
        ids[*ids_num] = 0x30280016;
        *ids_num += 1;

        ids[*ids_num] = 0x30083c;
        *ids_num += 1;

        if (g_scenarioState.completedLocationFlags[41] == 0)
        {
            ids[*ids_num] = 0x302907df;
            *ids_num += 1;
        }

        if (g_scenarioState.completedLocationFlags[42] == 0)
        {
            ids[*ids_num] = 0x302a083f;
            *ids_num += 1;
        }

        if (g_scenarioState.completedLocationFlags[43] == 0)
        {
            ids[*ids_num] = 0x302b07d4;
            *ids_num += 1;
        }
        break;

    case 50:
        ids[*ids_num] = 0x310017;
        *ids_num += 1;

        if (g_scenarioState.locationEntryStates[1] != 0 && g_scenarioState.completedLocationFlags[51] == 0)
        {
            ids[*ids_num] = 0x30330002;
            *ids_num += 1;
        }

        if (g_scenarioState.completedLocationFlags[53] == 0)
        {
            ids[*ids_num] = 0x303507e3;
            *ids_num += 1;
        }
        break;

    case 60:
    case 70:
    case 80:
        if (g_currentScenarioLocation->GetId() == 3 && 
            g_scenarioState.innSceneUnlockedLowConfidence != 0 && 
            g_scenarioState.completedLocationFlags[70] == 0)
        {
            ids[*ids_num] = 0x304602a8;
            *ids_num += 1;

            if (g_scenarioState.completedLocationFlags[71] == 0)
            {
                ids[*ids_num] = 0x304702a9;
                *ids_num += 1;
            }
        }

        if (g_currentScenarioLocation->GetId() == 3 && 
            g_scenarioState.scenarioChapter == 80 &&
            g_scenarioState.completedLocationFlags[83] == 0 && 
            g_scenarioState.locationEntryStates[4] != 0)
        {
            ids[*ids_num] = 0x305302a5;
            *ids_num += 1;
        }

        if (g_currentScenarioLocation->GetId() == 2 && g_scenarioState.scenarioChapter == 60)
        {
            if (g_scenarioState.completedLocationFlags[61] == 0)
            {
                ids[*ids_num] = 0x303d07d6;
                *ids_num += 1;
            }

            if (g_scenarioState.completedLocationFlags[63] == 0)
            {
                ids[*ids_num] = 0x303f083d;
                *ids_num += 1;
            }
        }

        if (g_currentScenarioLocation->GetId() == 2 && g_scenarioState.scenarioChapter == 70)
        {
            if (g_scenarioState.completedLocationFlags[73] == 0)
            {
                ids[*ids_num] = 0x304907d4;
                *ids_num += 1;
            }

            if (g_scenarioState.completedLocationFlags[72] == 0)
            {
                ids[*ids_num] = 0x3048083c;
                *ids_num += 1;
            }
        }

        if (g_scenarioState.scenarioChapter == 80 && g_currentScenarioLocation->GetId() == 2)
        {
            if (g_scenarioState.isMage == 0)
            {
                if (g_scenarioState.completedLocationFlags[82] == 0)
                {
                    ids[*ids_num] = 0x305207d9;
                    *ids_num += 1;
                }
            }
            else if (g_scenarioState.completedLocationFlags[81] == 0)
            {
                ids[*ids_num] = 0x305107da;
                *ids_num += 1;
            }
        }
        break;

    case 90:
        if (g_currentScenarioLocation->GetId() == 2)
        {
            ids[*ids_num] = 0x305a07d4;
            *ids_num += 1;
        }
        else if (g_currentScenarioLocation->GetId() == 3)
        {
            if (g_scenarioState.completedLocationFlags[91] == 0)
            {
                ids[*ids_num] = 0x305b02a9;
                *ids_num += 1;
            }

            if (g_scenarioState.completedLocationFlags[92] == 0)
            {
                ids[*ids_num] = 0x305c07d3;
                *ids_num += 1;
            }
        }
        break;

    case 100:
        if (g_currentScenarioLocation->GetId() == 2)
        {
            ids[*ids_num] = 0x306407d6;
            *ids_num += 1;

            if (g_scenarioState.completedLocationFlags[102] == 0)
            {
                ids[*ids_num] = 0x3066083d;
                *ids_num += 1;
            }

            if (g_scenarioState.completedLocationFlags[91] != 0 && g_scenarioState.completedLocationFlags[103] == 0)
            {
                ids[*ids_num] = 0x306707d5;
                *ids_num += 1;
            }
        }
        else if ((g_currentScenarioLocation->GetId() == 3) && (g_scenarioState.completedLocationFlags[0x65] == 0))
        {
            ids[*ids_num] = 0x306502a9;
            *ids_num += 1;
        }
        break;

    case 110:
        if (g_currentScenarioLocation->GetId() == 2)
        {
            ids[*ids_num] = 0x306e07d6;
            *ids_num += 1;
        }
        break;
    }

    if (g_scenarioState.scenarioChapter > 60 && g_scenarioState.laterSceneStateLowConfidence == 1 && g_currentScenarioLocation->GetId() == 3)
    {
        ids[*ids_num] = 0x4f0005;
        *ids_num += 1;
    }

    if (g_scenarioState.scenarioChapter > 60 && g_scenarioState.specialDialogStateLowConfidence == 1 && g_currentScenarioLocation->GetId() == 3)
    {
        ids[*ids_num] = 0x4e02a3;
        *ids_num += 1;
    }

    if (g_currentScenarioLocation->GetId() == 2)
    {
        if (g_scenarioState.completedLocationFlags[63] != 0 && 
            g_scenarioState.completedLocationFlags[74] == 0 &&
            g_scenarioState.completedLocationFlags[75] == 0 &&
            g_scenarioState.completedLocationFlags[76] == 0 && 
            g_scenarioState.completedLocationFlags[77] == 0)
        {
            if (g_scenarioState.isMage == 0)
            {
                if (g_scenarioState.isFemale == 0)
                {
                    ids[*ids_num] = 0x304a07e6;
                    *ids_num += 1;
                }
                else
                {
                    ids[*ids_num] = 0x304b07e6;
                    *ids_num += 1;
                }
            }
            else if (g_scenarioState.isFemale == 0)
            {
                ids[*ids_num] = 0x304d07e6;
                *ids_num += 1;
            }
            else
            {
                ids[*ids_num] = 0x304c07e6;
                *ids_num += 1;
            }
        }

        if ((g_scenarioState.completedLocationFlags[74] != 0 || g_scenarioState.completedLocationFlags[75] != 0 ||
            g_scenarioState.completedLocationFlags[76] != 0 || g_scenarioState.completedLocationFlags[77] != 0) &&
            g_scenarioState.completedLocationFlags[84] == 0 && 
            g_scenarioState.completedLocationFlags[85] == 0 &&
            g_scenarioState.completedLocationFlags[86] == 0 && 
            g_scenarioState.completedLocationFlags[87] == 0)
        {
            if (g_scenarioState.isMage == 0)
            {
                if (g_scenarioState.isFemale == 0)
                {
                    ids[*ids_num] = 0x305407e6;
                    *ids_num += 1;
                }
                else
                {
                    ids[*ids_num] = 0x305507e6;
                    *ids_num += 1;
                }
            }
            else if (g_scenarioState.isFemale == 0)
            {
                ids[*ids_num] = 0x305707e6;
                *ids_num += 1;
            }
            else
            {
                ids[*ids_num] = 0x305607e6;
                *ids_num += 1;
            }
        }

        if ((g_scenarioState.completedLocationFlags[84] != 0 || g_scenarioState.completedLocationFlags[85] != 0 ||
            g_scenarioState.completedLocationFlags[86] != 0 || g_scenarioState.completedLocationFlags[87] != 0) &&
            g_scenarioState.completedLocationFlags[94] == 0 && 
            g_scenarioState.completedLocationFlags[95] == 0 &&
            g_scenarioState.completedLocationFlags[96] == 0 && 
            g_scenarioState.completedLocationFlags[93] == 0)
        {
            if (g_scenarioState.isMage == 0)
            {
                if (g_scenarioState.isFemale == 0)
                {
                    ids[*ids_num] = 0x305d07e6;
                    *ids_num += 1;
                }
                else
                {
                    ids[*ids_num] = 0x305e07e6;
                    *ids_num += 1;
                }
            }
            else if (g_scenarioState.isFemale == 0)
            {
                ids[*ids_num] = 0x306007e6;
                *ids_num += 1;
            }
            else
            {
                ids[*ids_num] = 0x305f07e6;
                *ids_num += 1;
            }
        }
    }

    if (g_currentScenarioLocation->GetId() == 2 &&
        g_scenarioState.scenarioChapter > 59 &&
        g_scenarioState.completedLocationFlags[62] == 0 && 
        g_scenarioState.completedLocationFlags[53] != 0)
    {
        ids[*ids_num] = 0x303e0016;
        *ids_num += 1;
    }

    for (int i = 0; i < 20; i++)
    {
        if (g_scenarioState.locationEntryLocationIds[i] == g_currentScenarioLocation->GetId())
        {
            if (g_scenarioState.locationEntryStates[i] == 1)
            {
                ids[*ids_num] = (i + 1) | 0x10000000 | ((g_scenarioState.locationEntrySelectionFlags[i] > 0) << 31);
                *ids_num += 1;
            }
            else if (g_scenarioState.locationEntryStates[i] == 2)
            {
                ids[*ids_num] = (i + 1) | 0x20000000 | ((g_scenarioState.locationEntrySelectionFlags[i] > 0) << 31);
                *ids_num += 1;
            }
        }
    }
}


void __stdcall ScenarioEnterLocation(ScenarioLocation* loc)
{ //10001d22
    g_currentScenarioLocation = loc;
    for (int i = 0; i < 16; i++)
        g_scenarioState.currentLocationTransientFlags[i] = 0;
}

int32_t __stdcall ScenarioEnterShop()
{ //1000285f
    return 0;
}

CList<ScenarioLocation*>* __stdcall ScenarioGetAvailableLocations()
{ //100037de
    return &g_availableLocations;
}

CList<ScenarioLocation*>* __stdcall ScenarioGetAllLocations()
{ //100038d2
    return &g_allLocations;
}

ScenarioLocation* __stdcall ScenarioGetCurrentLocation()
{ //100038c8
    return g_currentScenarioLocation;
}

AssortGenParams* __stdcall ScenarioGetShopAssortment()
{ //100037e8
    return g_ShopsAssortment[g_currentScenarioLocation->GetId()];
}

int32_t __stdcall ScenarioGetVar(int32_t id)
{ //10001cfd
    return g_scenarioState.vars[id];
}

int32_t __stdcall ScenarioIsMissionAvailable(int32_t id)
{ //10003871
    for (POSITION pos = g_availableLocations.GetHeadPosition(); pos;)
    {
        ScenarioLocation* loc = g_availableLocations.GetNext(pos);
        if (loc->GetKind() == 1 && loc->GetId() == id)
            return 1;
    }
    return 0;
}

int32_t __stdcall ScenarioIsTownAvailable(int32_t id)
{ //10003800
    for (POSITION pos = g_availableLocations.GetHeadPosition(); pos;)
    {
        ScenarioLocation* loc = g_availableLocations.GetNext(pos);
        if (loc->GetKind() == 2)
        {
            if (id == 0 || loc->GetId() == id)
                return 1;
        }
    }
    return 0;
}


void __stdcall ScenarioLeaveInn()
{ //100032bb
}


void ScenarioMakeTownAvailable(int32_t id)
{ //1000199f
    for (POSITION pos = g_allLocations.GetHeadPosition(); pos;)
    {
        ScenarioLocation* loc = g_allLocations.GetNext(pos);
        if (loc->GetKind() == 2 && loc->GetId() == id)
        {
            POSITION p = g_availableLocations.Find(loc);
            if (!p)
                g_availableLocations.AddTail(loc);
        }
    }
}

void ScenarioMakeMissionAvailable(int32_t id)
{ //1000192f
    for (POSITION pos = g_allLocations.GetHeadPosition(); pos;)
    {
        ScenarioLocation* loc = g_allLocations.GetNext(pos);
        if (loc->GetKind() == 1 && loc->GetId() == id)
        {
            POSITION p = g_availableLocations.Find(loc);
            if (!p)
                g_availableLocations.AddTail(loc);
        }
    }
}


int32_t ScenarioOnCompleted(int32_t* loc, int32_t id)
{
    g_scenarioState.locationTransitionStateLowConfidence = 0;

    if (g_scenarioState.rebuildAvailableLocationsPending != 0)
    {
        g_scenarioState.rebuildAvailableLocationsPending = 0;
        g_availableLocations.RemoveAll();
        g_availableLocations.AddTail(&g_scenarioLocationTown1_2);
        g_availableLocations.AddTail(&g_scenarioLocationTown3);
        g_availableLocations.AddTail(&g_scenarioLocationMission21);
        g_availableLocations.AddTail(&g_scenarioLocationMission30);
        g_availableLocations.AddTail(&g_scenarioLocationMission31);
        g_availableLocations.AddTail(&g_scenarioLocationMission32);
        g_availableLocations.AddTail(&g_scenarioLocationMission40);
        g_availableLocations.AddTail(&g_scenarioLocationMission41);
        g_availableLocations.AddTail(&g_scenarioLocationMission42);
        g_availableLocations.AddTail(&g_scenarioLocationMission43);
        g_availableLocations.AddTail(&g_scenarioLocationMission50);
        g_availableLocations.AddTail(&g_scenarioLocationMission51);
        g_availableLocations.AddTail(&g_scenarioLocationMission52);
        g_availableLocations.AddTail(&g_scenarioLocationMission53);
        g_availableLocations.AddTail(&g_scenarioLocationMission60);
        g_availableLocations.AddTail(&g_scenarioLocationMission61);
        g_availableLocations.AddTail(&g_scenarioLocationMission62);
        g_availableLocations.AddTail(&g_scenarioLocationMission63);
        g_availableLocations.AddTail(&g_scenarioLocationMission70);
        g_availableLocations.AddTail(&g_scenarioLocationMission71);
        g_availableLocations.AddTail(&g_scenarioLocationMission72);
        g_availableLocations.AddTail(&g_scenarioLocationMission73);
        g_availableLocations.AddTail(&g_scenarioLocationMission74);
        g_availableLocations.AddTail(&g_scenarioLocationMission75);
        g_availableLocations.AddTail(&g_scenarioLocationMission76);
        g_availableLocations.AddTail(&g_scenarioLocationMission77);
        g_availableLocations.AddTail(&g_scenarioLocationMission80);
        g_availableLocations.AddTail(&g_scenarioLocationMission81);
        g_availableLocations.AddTail(&g_scenarioLocationMission82);
        g_availableLocations.AddTail(&g_scenarioLocationMission83);
        g_availableLocations.AddTail(&g_scenarioLocationMission84);
        g_availableLocations.AddTail(&g_scenarioLocationMission85);
        g_availableLocations.AddTail(&g_scenarioLocationMission86);
        g_availableLocations.AddTail(&g_scenarioLocationMission87);
        g_availableLocations.AddTail(&g_scenarioLocationMission90);
        g_availableLocations.AddTail(&g_scenarioLocationMission91);
        g_availableLocations.AddTail(&g_scenarioLocationMission92);

        if (g_scenarioState.isMage == 0)
        {
            if (g_scenarioState.isFemale == 0)
                g_availableLocations.AddTail(&g_scenarioLocationMission93);
            else
                g_availableLocations.AddTail(&g_scenarioLocationMission94);
        }
        else if (g_scenarioState.isFemale == 0)
            g_availableLocations.AddTail(&g_scenarioLocationMission96);
        else
            g_availableLocations.AddTail(&g_scenarioLocationMission95);

        g_availableLocations.AddTail(&g_scenarioLocationMission100);
        g_availableLocations.AddTail(&g_scenarioLocationMission101);
        g_availableLocations.AddTail(&g_scenarioLocationMission102);
        g_availableLocations.AddTail(&g_scenarioLocationMission103);
        g_availableLocations.AddTail(&g_scenarioLocationMission110);
    }

    for (int i = 0; i < 20; i++)
    {
        if (g_scenarioState.locationEntryStates[i] != 0)
        {
            if (g_scenarioState.locationEntrySelectionFlags[i] == 0)
                g_scenarioState.locationEntryStates[i] = 1;
            else
                g_scenarioState.locationEntryStates[i] = 2;
        }

        g_scenarioState.locationEntrySelectionFlags[i] = 0;
    }


    POSITION pos = g_availableLocations.Find(g_currentScenarioLocation);
    if (pos)
        g_availableLocations.RemoveAt(pos);

    g_currentScenarioLocation = nullptr;
    g_scenarioState.completedLocationFlags[id] = 1;

    switch (id)
    {
    case 10:
        ScenarioMakeMissionAvailable(20);
        *loc = 1;
        break;

    case 20:
        ScenarioMakeTownAvailable(2);
        if (g_scenarioState.followupLocationUnlockFlagLowConfidence != 0)
            ScenarioMakeMissionAvailable(21);
        g_scenarioState.locationEntryStates[0] = 1;
        g_scenarioState.locationEntryLocationIds[0] = 2;
        break;

    case 30:
        g_ShopsAssortment[2][0].flags = 0x1383c22c;
        g_ShopsAssortment[2][1].flags = 0x2bc3c324;
        g_ShopsAssortment[2][3].flags = 0x10438324;
        *loc = 2;
        break;

    case 31:
        ScenarioMakeMissionAvailable(32);
        break;

    case 40:
        ScenarioMakeMissionAvailable(50);
        ScenarioMakeMissionAvailable(60);
        g_scenarioState.locationTransitionStateLowConfidence = 23;
        g_ShopsAssortment[2][0].flags = 0x1387c26c;
        g_ShopsAssortment[2][1].flags = 0x2bc7c324;
        g_ShopsAssortment[2][3].flags = 0x10478264;
        break;

    case 50:
        ScenarioMakeTownAvailable(3);
        if (g_scenarioState.mission31UnlockedLowConfidence != 0)
            g_availableLocations.AddTail(&g_scenarioLocationMission52);

        g_scenarioState.innSceneUnlockedLowConfidence = 1;

        for (int i = 0; i < 4; i++)
        {
            if (i == 2)
                g_ShopsAssortment[3][2].min_cost = 0;
            else
                g_ShopsAssortment[3][i].min_cost = 499;
            
            if (i == 0 || i == 3)
                g_ShopsAssortment[3][i].max_count = 100;
            else
                g_ShopsAssortment[3][i].max_count = 20;

            if (i == 0 || i == 3)
                g_ShopsAssortment[3][i].max_same_count = 2;
            else
                g_ShopsAssortment[3][i].max_same_count = 1;
        }

        g_ShopsAssortment[3][0].flags = 0x13c7d318;
        g_ShopsAssortment[3][1].flags = 0x2bc7d318;
        g_ShopsAssortment[3][2].flags = 0x4000000;
        g_ShopsAssortment[3][3].flags = 0x2bc042f0;
        g_scenarioState.locationEntryStates[2] = 1;
        g_scenarioState.locationEntryLocationIds[2] = 3;
        break;

    case 60:
        ScenarioMakeMissionAvailable(80);
        g_scenarioState.locationEntryStates[5] = 1;
        g_scenarioState.locationEntryLocationIds[5] = 2;
        g_scenarioState.laterSceneStateLowConfidence = 1;
        g_ShopsAssortment[2][0].flags = 0x1387c268;
        g_ShopsAssortment[2][1].flags = 0x2bc7c268;
        g_ShopsAssortment[2][3].flags = 0x10478260;
        break;

    case 70:
        g_scenarioState.locationEntryStates[3] = 1;
        g_scenarioState.locationEntryLocationIds[3] = 3;
        if (g_scenarioState.unk == 0 && g_scenarioState.pairedObjectiveBCompletedLowConfidence != 0)
            *loc = 3;
        
        g_scenarioState.unk = 1;
        g_scenarioState.specialDialogStateLowConfidence = 1;
        break;

    case 80:
        if (g_scenarioState.pairedObjectiveBCompletedLowConfidence == 0 && g_scenarioState.unk != 0)
            *loc = 3;

        g_scenarioState.pairedObjectiveBCompletedLowConfidence = 1;
        g_ShopsAssortment[2][0].flags = 0x138752e0;
        g_ShopsAssortment[2][1].flags = 0x2bc752e0;
        g_ShopsAssortment[2][3].flags = 0x104702e0;
        break;

    case 90:
        g_ShopsAssortment[2][0].flags = 0x138772c0;
        g_ShopsAssortment[2][1].flags = 0x2bc772c0;
        g_ShopsAssortment[2][3].flags = 0x104722c0;
        break;

    case 100:
        g_scenarioState.locationEntryStates[6] = 1;
        g_scenarioState.locationEntryLocationIds[6] = 2;
        break;

    case 110:
        if (g_scenarioState.endingVariantFlagLowConfidence == 0)
            *loc = 4;
        else
            *loc = 5;
        break;
    }

    if (id % 10 == 0)
        g_scenarioState.scenarioChapter += 10;
    
    switch (g_scenarioState.scenarioChapter)
    {
    case 30:
        for (int i = 0; i < 4; i++)
            g_ShopsAssortment[2][i].max_cost = 10000;
        break;

    case 40:
        for (int i = 0; i < 4; i++)
            g_ShopsAssortment[2][i].max_cost = 22000;
        break;

    case 50:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 60000;
            g_ShopsAssortment[2][i].max_cost = 60000;
        }
        break;

    case 60:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 150000;
            g_ShopsAssortment[2][i].max_cost = 150000;
        }
        break;

    case 70:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 400000;
            g_ShopsAssortment[2][i].max_cost = 400000;
        }
        break;

    case 80:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 800000;
            g_ShopsAssortment[2][i].max_cost = 800000;
        }
        break;

    case 90:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 1500000;
            g_ShopsAssortment[2][i].max_cost = 1500000;
            if (i != 2)
                g_ShopsAssortment[2][i].min_cost = 12000;
        }
        break;

    case 100:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 5000000;
            g_ShopsAssortment[2][i].max_cost = 5000000;
            if (i != 2)
                g_ShopsAssortment[2][i].min_cost = 40000;
        }
        break;

    case 110:
        for (int i = 0; i < 4; i++)
        {
            g_ShopsAssortment[3][i].max_cost = 10000000;
            g_ShopsAssortment[2][i].max_cost = 10000000;
        }
        break;
    }
    return 0;
}


int32_t __stdcall ScenarioLeaveLocation(int32_t* loc)
{ //10002806
    *loc = -1;

    if (!g_currentScenarioLocation)
        return 0;

    if (g_currentScenarioLocation->GetKind() == 2)
    {
        if (g_currentScenarioLocation->GetId() == 1)
        {
            POSITION pos = g_availableLocations.Find(g_currentScenarioLocation);
            if (pos)
                g_availableLocations.RemoveAt(pos);
        }
        g_currentScenarioLocation = nullptr;
        return 0;
    }

    return ScenarioOnCompleted(loc, g_currentScenarioLocation->GetId());
}

void __stdcall ScenarioLeaveShop()
{ // 10002866
}

void __stdcall ScenarioLoad(CFile* fl)
{ //10003686
    fl->Read(&g_scenarioState, 0x1000);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            fl->Read(&g_ShopsAssortment[i][j], 20);
    }

    int32_t num = 0;
    fl->Read(&num, 4);

    g_availableLocations.RemoveAll();

    InitAllScenatioLocations();

    for (int i = 0; i < num; i++)
    {
        ScenarioLocation loc;
        loc.Read(fl);

        if (loc.GetKind() == 1)
            ScenarioMakeMissionAvailable(loc.GetId());
        else
            ScenarioMakeTownAvailable(loc.GetId());
    }

    fl->Read(&num, 4);

    POSITION pos = g_availableLocations.GetHeadPosition();
    for (int i = 0; i != num; i++)
         g_availableLocations.GetNext(pos);

    g_currentScenarioLocation = g_availableLocations.GetAt(pos);
}

void __stdcall ScenarioSave(CFile* fl)
{ //10003585
    fl->Write(&g_scenarioState, 0x1000);
    for (int i = 0; i < 4; i += 1)
    {
        for (int j = 0; j < 4; j += 1)
            fl->Write(&g_ShopsAssortment[i][j], 20);
    }

    int32_t num = g_availableLocations.GetCount();
    fl->Write(&num, 4);

    int i = 0;
    for (POSITION pos = g_availableLocations.GetHeadPosition(); pos; i++)
    {
        ScenarioLocation* loc = g_availableLocations.GetNext(pos);
        if (g_currentScenarioLocation == loc)
            num = i;
    }

    fl->Write(&num, 4);
}



void __stdcall ScenarioNewGame()
{ //100033be
    memset(&g_scenarioState, 0, 0x1000);

    InitAllScenatioLocations();

    for (int i = 0; i < 20; i++)
        g_scenarioState.locationEntryStates[i] = 0;

    g_scenarioState.scenarioChapter = 10;

    g_availableLocations.RemoveAll();
    g_availableLocations.AddTail(&g_scenarioLocationTown1_1);
    
    ScenarioEnterLocation(&g_scenarioLocationTown1_1);

    for (int i = 0; i < 4; i++)
    {
        g_ShopsAssortment[1][i].min_cost = 0;
        g_ShopsAssortment[1][i].max_cost = 1500;
        g_ShopsAssortment[1][i].max_count = ((1 < i) - 1 & 0x50) + 20;
        g_ShopsAssortment[1][i].max_same_count = (i < 2) + 1;
    }

    g_ShopsAssortment[1][0].flags = 0x1381cc03;
    g_ShopsAssortment[1][1].flags = 0x10418103;
    g_ShopsAssortment[1][2].flags = 0x28018100;
    g_ShopsAssortment[1][3].flags = 0x4000000;

    for (int i = 0; i < 4; i++)
    {
        g_ShopsAssortment[2][i].min_cost = 0;
        g_ShopsAssortment[2][i].max_cost = 5000;

        if (i == 0 || i == 3)
            g_ShopsAssortment[2][i].max_count = 100;
        else
            g_ShopsAssortment[2][i].max_count = 20;

        if (i == 0 || i == 3)
            g_ShopsAssortment[2][i].max_same_count = 2;
        else
            g_ShopsAssortment[2][i].max_same_count = 1;
    }

    g_ShopsAssortment[2][0].flags = 0x1383c305;
    g_ShopsAssortment[2][1].flags = 0x2bc3c304;
    g_ShopsAssortment[2][2].flags = 0x4000000;
    g_ShopsAssortment[2][3].flags = 0x10438305;
}


void __stdcall ScenarioSetVar(int32_t id, int32_t value)
{ //10001d0e
    g_scenarioState.vars[id] = value;
}

void __stdcall ScenarioTalkTo(uint32_t t)
{ //100032c0
    uint32_t entry = t & 0xffff;
    uint32_t id = (t >> 16) & 0xfff;
    uint32_t kind = (t >> 28) & 7;

    if (kind == 0)
    {
        if (entry == 5)
        {
            g_scenarioState.locationEntryStates[4] = 1;
            g_scenarioState.locationEntryLocationIds[4] = 3;
            g_scenarioState.laterSceneStateLowConfidence = 2;
        }

        if (entry == 675 && id == 78)
        {
            g_scenarioState.specialDialogStateLowConfidence = 2;
            g_ShopsAssortment[3][3].flags |= 0x80000;
        }
    }
    else if (kind == 1)
    {
        g_scenarioState.locationEntrySelectionFlags[entry - 1] = g_scenarioState.locationEntrySelectionFlags[entry - 1] == 0 ? 1 : 0;
    }
    else if (kind == 3)
    {
        ScenarioMakeMissionAvailable(id);

        if (id == g_scenarioLocationMission10.GetId())
            g_scenarioState.startMissionAllowed = 1;

        if (entry == 22 && id == 30)
        {
            g_scenarioState.locationEntryStates[1] = 1;
            g_scenarioState.locationEntryLocationIds[1] = 2;
        }
    }
}


void ScenarioLocation::Read(CFile* fl)
{ //10003a20
    fl->Read(&id, 4);
    fl->Read(&kind, 4);
    fl->Read(&rect, sizeof(CRect));
}

void ScenarioLocation::Write(CFile* fl)
{ //10003990
    fl->Write(&id, 4);
    fl->Write(&kind, 4);
    fl->Write(&rect, sizeof(CRect));
}
