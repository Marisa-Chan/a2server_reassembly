#include <winsock2.h>
#include "patch/locale.h"
#include <ws2tcpip.h>
#include <timeapi.h>

#include "server.h"
#include "main_window.h"
#include "gfx.h"
#include "mouse.h"
#include "file.h"
#include "game_app.h"
#include "quest_map.h"
#include "quest.h"
#include "ingame.h"
#include "util.h"
#include "resource.h"
#include "spell.h"
#include "players_list.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "buildings_list.h"
#include "packet.h"
#include "vis_worldmap.h"

// Statics for VisGlobalMap::MsgProc (65FA30, 65FA68 in the binary).
static uint8_t gmap_repaint_init = 0;     // 65FA30 init-done flag
static uint32_t gmap_repaint_ts = 0;      // 65FA68 timestamp of the last throttled repaint

void VisGlobalMap::RebuildScenarioLocations() { //47024a
    this->PopulateScenarioLocationFlags();
}

void VisGlobalMap::PopulateScenarioLocationFlags() { //47025d
    CList<ScenarioLocation*> *locs = ScenarioGetAllLocations();
    for (POSITION pos = locs->GetHeadPosition(); pos != nullptr;) {
        ScenarioLocation* loc = locs->GetNext(pos);
        this->locationPoints.Add(loc->GetRect().TopLeft());

        if (loc->GetRect().BottomRight() == CPoint(0, 0)) {
            this->locationAvailabilityFlags.Add(1);
        }
    }
}


// 4714E7
void VisGlobalMap::VMethod7() {
    CPoint screen_pt = this->rect.TopLeft();
    int32_t screen_x = screen_pt.x;
    int32_t screen_y = screen_pt.y;
    int32_t travel_arrived = 0;
    int32_t arrival_index = 0;

    if (this->renderActiveFlag == 0) {
        return;
    }

    LockSurface2();
    this->gmap->Draw(screen_x, screen_y, 0, 0, 0);

    if (this->umoirMapMode == 0) {
        if (this->travelRoutePoints.GetSize() == 0 && this->travelProgress == 0) {
            this->OnMapClick();
        }
    } else {
        if (this->travelProgress < 8) {
            ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetHead();
            CPoint loc_pt = loc->GetRect().TopLeft();
            this->currentLocationPoint = loc_pt;
            this->targetLocationPoint = loc_pt;
            this->ComputeTravelRoute(this->currentLocationPoint.x, this->currentLocationPoint.y,
                this->targetLocationPoint.x, this->targetLocationPoint.y);
            this->travelProgress = 8;
        }
    }

    if (this->travelProgress == 0) {
        for (int32_t i = 0; i < this->travelRoutePoints.GetSize(); i++) {
            if (i % 8 == 0) {
                CPoint& pt = this->travelRoutePoints.ElementAt(i);
                int32_t w = this->ballmap->GetWidth(0);
                int32_t h = this->ballmap->GetHeight(0);
                this->ballmap->DrawSubRectMasked(screen_x + pt.x - w / 2, screen_y + pt.y - h / 2, 0, 0, w, h);
            }
        }
        for (POSITION pos = ScenarioGetAvailableLocations()->GetHeadPosition(); pos != nullptr;) {
            ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetNext(pos);
            CPoint loc_pt = loc->GetRect().TopLeft();
            if (this->umoirMapMode == 0) {
                if (this->currentLocationPoint != CPoint(loc_pt.x, loc_pt.y)) {
                    if (this->targetLocationPoint == CPoint(loc_pt.x, loc_pt.y)) {
                        this->mission_flg->Draw(screen_x - 5 + loc_pt.x, screen_y - 0x29 + loc_pt.y,
                            this->mapFlagAnimationFrame, 0, 0);
                    } else {
                        this->flg_on_map->Draw(screen_x - 5 + loc_pt.x, screen_y - 0x25 + loc_pt.y,
                            this->mapFlagAnimationFrame, 0, 0);
                    }
                }
            } else {
                this->flag1_spr->Draw(screen_x - 4 + loc_pt.x, screen_y - 0x20 + loc_pt.y,
                    this->mapFlagAnimationFrame, 0, 0);
            }
        }
    } else {
        int32_t dot_count = this->travelProgress;
        if (dot_count >= this->travelRoutePoints.GetUpperBound()) {
            dot_count = this->travelRoutePoints.GetUpperBound();
        }
        for (int32_t i = 0; i < dot_count; i++) {
            if (i % 8 == 0) {
                CPoint& pt = this->travelRoutePoints.ElementAt(i);
                int32_t w = this->ballmap->GetWidth(0);
                int32_t h = this->ballmap->GetHeight(0);
                this->ballmap->DrawSubRectMasked(screen_x + pt.x - w / 2, screen_y + pt.y - h / 2, 0, 0, w, h);
            }
        }
        int32_t cross_frames = this->cross_spr->GetFrameCount();
        if (this->targetCrossAnimationFrame < cross_frames - 1) {
            this->cross_spr->Draw(screen_x - 10 + this->targetLocationPoint.x,
                screen_y - 0xC + this->targetLocationPoint.y, this->targetCrossAnimationFrame, 0, 0);
        } else {
            this->cross_spr->Draw(screen_x - 10 + this->targetLocationPoint.x,
                screen_y - 0xC + this->targetLocationPoint.y, cross_frames - 1, 0, 0);
        }
        this->targetCrossAnimationFrame++;
        this->travelProgress += 8;
        if (this->travelProgress < this->travelRoutePoints.GetSize()) {
            CSound& cur_snd = (this->routePointSoundIndex == 0) ? this->snd_point1 : this->snd_point2;
            if (!IsSfxPlaying(&cur_snd)) {
                this->routePointSoundIndex = (this->routePointSoundIndex + 1) & 1;
                CSound& new_snd = (this->routePointSoundIndex == 0) ? this->snd_point1 : this->snd_point2;
                CSound::Play(new_snd);
            }
        }
        if (this->travelProgress > this->travelRoutePoints.GetSize()
            && this->targetCrossAnimationFrame > this->cross_spr->GetFrameCount() + 1) {
            this->currentLocationPoint = this->targetLocationPoint;
            this->travelProgress = 0;
            this->travelRoutePoints.RemoveAll();
            int32_t i;
            for (i = 0; i < this->locationPoints.GetSize(); i++) {
                CPoint& pt = this->locationPoints.ElementAt(i);
                if (pt == this->targetLocationPoint) {
                    break;
                }
            }
            travel_arrived = 1;
            arrival_index = i;
        }
    }

    this->mapFlagAnimationFrame = (this->mapFlagAnimationFrame + 1) % this->flag1_spr->GetFrameCount();
    if (this->umoirMapMode != 0) {
        this->flag_spr->Draw(screen_x - 0xA + this->currentLocationPoint.x,
            screen_y - 0x18 + this->currentLocationPoint.y, this->partyFlagAnimationFrame, 0, 0);
    } else {
        this->yflag_spr->Draw(screen_x - 0x14 + this->currentLocationPoint.x,
            screen_y - 0x2C + this->currentLocationPoint.y, this->partyFlagAnimationFrame, 0, 0);
    }
    this->partyFlagAnimationFrame = (this->partyFlagAnimationFrame + 1) % this->flag_spr->GetFrameCount();

    if (this->umoirMapMode == 0 && !this->hoveredLocationTitle.IsEmpty()) {
        g_font4->DrawTxt(screen_x + 0xE6, screen_y + 0x1B, this->hoveredLocationTitle, 2,
            palette_brown_derby->GetPalette(0));
        int32_t upper = this->hoveredLocationLines.GetUpperBound();
        for (int32_t i = 0; i < upper; i++) {
            g_font2->DrawTxt(screen_x + 0xE6, screen_y + 0x30 + i * 10,
                this->hoveredLocationLines.GetAt(i), 2, clrsh_InvBarleyCorn);
        }
        const CString& last_line = this->hoveredLocationLines.GetAt(upper);
        CString trimmed = last_line.Left(last_line.GetLength() - 2);
        g_font2->DrawTxt(screen_x + 0xE6, screen_y + 0x30 + upper * 10, trimmed, 2, clrsh_InvBarleyCorn);
    }

    UnlockSurface2();
    CVisualObject::VMethod7();
    if (travel_arrived != 0) {
        this->MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        AfxGetMainWnd()->PostMessage(MAIN_MSG_ENTER_SCENARIO_LOCATION, 0, 0);
        CList<ScenarioLocation*>* locs = ScenarioGetAvailableLocations();
        if (this->umoirMapMode == 0) {
            for (POSITION pos = locs->GetHeadPosition(); pos != nullptr;) {
                ScenarioLocation* loc = locs->GetNext(pos);
                CPoint loc_pt = loc->GetRect().TopLeft();
                if (this->targetLocationPoint == CPoint(loc_pt.x, loc_pt.y)) {
                    ScenarioEnterLocation(loc);
                    break;
                }
            }
        } else {
            ScenarioEnterLocation(locs->GetHead());
        }
    }
    this->UpdateHoveredLocation(CPoint(g_mousept.GetX(), g_mousept.GetY()));
}


// 471367
void VisGlobalMap::DoClose(uint32_t code) {
    this->renderActiveFlag = 0;
    this->FreeBitmaps();
    this->FreeSamples();
    if (this->routeAdjacencyMatrix != nullptr) {
        void** rows = static_cast<void**>(this->routeAdjacencyMatrix);
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            CArray<CPoint>** row = static_cast<CArray<CPoint>**>(rows[i]);
            for (int32_t j = 0; j < this->graphNodePoints.GetSize(); j++) {
                if (row[j] != nullptr) {
                    delete row[j];
                }
            }
        }
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            operator delete(rows[i]);
        }
        operator delete(rows);
    }
    this->routeAdjacencyMatrix = nullptr;
    this->graphNodePoints.RemoveAll();
    this->travelRoutePoints.RemoveAll();
    VisScreen::DoClose(code);
}


// 473195
GMapThing::GMapThing() {
    for (int32_t x = 0; x < 640; x++) {
        for (int32_t y = 0; y < 480; y++) {
            this->visitedGrid[x][y] = 0;
        }
    }
    this->pathmap = nullptr;
    this->LoadPathmap();
    this->bmp_data = static_cast<uint8_t*>(this->pathmap->GetData());
    this->nodeCount = 0;
    for (int32_t x = 0; x < 640; x++) {
        for (int32_t y = 0; y < 480; y++) {
            int32_t offset = y * 640 + x;
            if (this->bmp_data[offset] == 2) {
                this->nodeCount++;
                this->nodePoints.Add(CPoint(x, y));
            }
        }
    }
    this->adjacencyMatrix = static_cast<CArray<CPoint>***>(operator new(this->nodeCount * 4));
    for (int32_t i = 0; i < this->nodeCount; i++) {
        this->adjacencyMatrix[i] = static_cast<CArray<CPoint>**>(operator new(this->nodeCount * 4));
    }
    for (int32_t i = 0; i < this->nodeCount; i++) {
        for (int32_t j = 0; j < this->nodeCount; j++) {
            this->adjacencyMatrix[i][j] = nullptr;
        }
    }
    this->BuildAdjacency();
}


// 47362d
void GMapThing::BuildAdjacency() {
    CArray<CPoint> foundPoints;
    CArray<CPoint> reverseBuf;
    for (int32_t i = 0; i < this->nodeCount; i++) {
        foundPoints.RemoveAll();
        foundPoints.Add(this->nodePoints[i]);
        CPoint node = this->nodePoints[i];
        this->visitedGrid[node.x][node.y] = 1;
        this->floodPos = node;
        int32_t size;
        do {
            this->FloodFill(this->floodPos.x, this->floodPos.y, 2, &foundPoints);
            size = foundPoints.GetSize();
            if (size != 1) {
                CPoint last = foundPoints.GetAt(foundPoints.GetUpperBound());
                for (int32_t j = 0; j < this->nodeCount; j++) {
                    if (this->nodePoints[j] == last) {
                        if (this->adjacencyMatrix[i][j] == nullptr) {
                            this->adjacencyMatrix[i][j] = new CArray<CPoint>();
                            this->adjacencyMatrix[i][j]->InsertAt(0, &foundPoints);
                            reverseBuf.RemoveAll();
                            while (foundPoints.GetSize() != 0) {
                                reverseBuf.InsertAt(0, foundPoints.GetAt(0), 1);
                                foundPoints.RemoveAt(0, 1);
                            }
                            this->adjacencyMatrix[j][i] = new CArray<CPoint>();
                            this->adjacencyMatrix[j][i]->InsertAt(0, &reverseBuf);
                            foundPoints.Add(this->nodePoints[i]);
                        }
                        break;
                    }
                }
            }
        } while (size > 1);
    }
}


// 4733cf
GMapThing::~GMapThing() {
    this->FreePathmap();
    if (this->adjacencyMatrix != nullptr) {
        for (int32_t i = 0; i < this->nodeCount; i++) {
            for (int32_t j = 0; j < this->nodeCount; j++) {
                CArray<CPoint>* cell = this->adjacencyMatrix[i][j];
                if (cell != nullptr) {
                    delete cell;
                }
            }
        }
        for (int32_t i = 0; i < this->nodeCount; i++) {
            operator delete(this->adjacencyMatrix[i]);
        }
        operator delete(this->adjacencyMatrix);
    }
}


// 47391f
int32_t GMapThing::FloodFill(int32_t x, int32_t y, uint16_t mark, CArray<CPoint>* foundPoints) {
    int32_t centerOffset = y * 640 + x;
    for (int32_t nx = x - 1; nx <= x + 1; nx++) {
        if (nx < 0 || nx >= 640) {
            continue;
        }
        for (int32_t ny = y - 1; ny <= y + 1; ny++) {
            if (ny < 0 || ny >= 480) {
                continue;
            }
            CPoint pt(nx, ny);
            if (pt == CPoint(x, y)) {
                continue;
            }
            int32_t offset = ny * 640 + nx;
            if (this->bmp_data[offset] == 1) {
                if (this->visitedGrid[nx][ny] == 0) {
                    this->visitedGrid[nx][ny] = mark;
                    foundPoints->Add(CPoint(nx, ny));
                    if (this->FloodFill(nx, ny, mark + 1, foundPoints) == 0) {
                        return 0;
                    }
                }
            } else if (this->bmp_data[offset] == 2) {
                if (this->floodPos != CPoint(nx, ny)) {
                    foundPoints->Add(CPoint(nx, ny));
                    return 0;
                }
            } else if (this->bmp_data[offset] == 1 && this->bmp_data[centerOffset] == 2 && this->visitedGrid[nx][ny] != 0) {
                // already-visited cell next to a node: nothing to do
            }
        }
    }
    return 0;
}


// 473508
void GMapThing::LoadPathmap() {
    MainWindow* mainWnd = (MainWindow*)AfxGetMainWnd();
    this->FreePathmap();
    if (mainWnd->vis_globalmap->umoirMapMode != 0) {
        this->pathmap = new CBmp256("graphics\\Global.Map\\UmoirPath.bmp");
    } else {
        this->pathmap = new CBmp256("graphics\\Global.Map\\PathMap.bmp");
    }
}


// 4735e2
void GMapThing::FreePathmap() {
    if (this->pathmap != nullptr) {
        delete this->pathmap;
    }
    this->pathmap = nullptr;
}


// 473b4a
void* GMapThing::TakeAdjacency(void* nodePoints) {
    CArray<CPoint>*** matrix = this->adjacencyMatrix;
    this->adjacencyMatrix = nullptr;
    static_cast<CArray<CPoint>*>(nodePoints)->InsertAt(0, &this->nodePoints);
    return matrix;
}


// 47115D
void VisGlobalMap::VMethod28() {
    g_mousept.DisableHint();
    this->LoadBitmaps();
    this->LoadSamples();
    ReadFileToString("main\\text\\globalmap.txt", &g_MissionText);
    this->routePointSoundIndex = 0;
    GMapThing* thing = new GMapThing();
    this->graphNodePoints.RemoveAll();
    this->routeAdjacencyMatrix = thing->TakeAdjacency(&this->graphNodePoints);
    delete thing;
    this->partyFlagAnimationFrame = 0;
    this->targetCrossAnimationFrame = 0;
    this->locationMetadata.RemoveAll();
    LockSurface2();
    FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    UnlockSurface2();
    FlushScreen();
    VisScreen::VMethod28();
    g_Cursors[CURSOR_SELECT]->Use();
    this->renderActiveFlag = 1;
    g_mousept.EnableHint();
}


// 47031D
void VisGlobalMap::VMethod26() {
    this->hero_bmp = nullptr;
    this->routeAdjacencyMatrix = nullptr;
    this->ballmap = nullptr;
    this->flag1_spr = nullptr;
    this->flag_spr = nullptr;
    this->cross_spr = nullptr;
    this->flg_on_map = nullptr;
    this->yflag_spr = nullptr;
    this->scroll1_bmp = nullptr;
    this->scrollp1_bmp = nullptr;
    this->gmap = nullptr;
    this->scroll3_bmp = nullptr;
    this->mission_flg = nullptr;
    this->scrollp2_bmp = nullptr;
    this->snd_scrollup.sample = nullptr;
    this->snd_point1.sample = nullptr;
    this->snd_point2.sample = nullptr;
    this->scroll2_bmp = nullptr;
    this->scrollp3_bmp = nullptr;
    this->snd_scrolldn.sample = nullptr;
    this->partDetailsRect.SetRectEmpty();
    this->mapFlagAnimationFrame = 0;
    this->travelProgress = 0;
    this->hoveredLocationIndex = -1;
    this->heroBitmapSize = CSize(0xA, 0x14);

    this->AddChild(new VisButton(4, 0x21C, 0x1C2, 0x21C, 0x1C2, " ", g_font1, clrsh_TechBlack, 0x7FFF, 0, nullptr));
}


// 472729
const char* VisGlobalMap::GetHint() {
    if (this->renderActiveFlag == 0) {
        return nullptr;
    }
    if (!this->partDetailsRect.IsRectNull()) {
        return nullptr;
    }
    CPoint pt(g_mousept.GetX() - this->rect.left, g_mousept.GetY() - this->rect.top);
    for (int32_t i = 0; i < this->locationHitRects.GetSize(); i++) {
        if (this->locationHitRects.ElementAt(i).PtInRect(pt)) {
            if (this->locationAvailabilityFlags.ElementAt(i) == 0) {
                return nullptr;
            }
            return g_GlobalMapLocationHints.GetLine(i);
        }
    }
    return nullptr;
}


// 473077
int32_t VisGlobalMap::OnRButtonDown(uint32_t wparam, CPoint pos) {
    // Binary builds the popup rect from heroDrawPoint, which nothing ever
    // writes (stays (0,0) from the ctor) — vanilla quirk, kept faithfully.
    CRect popup_rect(CPoint(this->heroDrawPoint), this->heroBitmapSize);
    CPoint screen_pt = this->rect.TopLeft();
    pos -= CSize(screen_pt.x, screen_pt.y);
    if (popup_rect.PtInRect(pos)) {
        int32_t count = g_StructEnter.structure_list.GetSize();
        if (count < 2) {
            count = 2;
        }
        this->partDetailsRect = CRect(0x64, 0x64, 0x15E, count * 0x14 + 0x78);
    }
    return 1;
}

// 472CC5
int32_t VisGlobalMap::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if ((gmap_repaint_init & 1) == 0) {
        gmap_repaint_init |= 1;
        gmap_repaint_ts = timeGetTime() - 100;
    }
    if (msg == VIS_MSG_FRAME_TICK) {
        if (timeGetTime() - gmap_repaint_ts >= 100) {
            this->VMethod9();
            gmap_repaint_ts = timeGetTime();
        }
    }
    return VisScreen::MsgProc(msg, wparam, lparam);
}


// 473038
int32_t VisGlobalMap::OnLButtonDown(uint32_t wparam, CPoint pos) {
    if (this->travelProgress == 0) {
        this->travelProgress = 8;
    } else {
        this->FinishTravel();
    }
    return 1;
}


// 472D99
int32_t VisGlobalMap::OnMouseMove(uint32_t wparam, CPoint pos) {
    if (this->travelProgress == 0) {
        this->OnMapClick();
    }
    this->UpdateHoveredLocation(pos);
    return 0;
}


// 473175
int32_t VisGlobalMap::OnRButtonUp(uint32_t wparam, CPoint pos) {
    this->partDetailsRect.SetRectEmpty();
    return 1;
}


// 4726CC
int32_t VisGlobalMap::OnChar(uint32_t wparam) {
    this->FinishTravel();
    return 1;
}


// 4726E6
int32_t VisGlobalMap::OnKeyDown(uint32_t wparam) {
    this->FinishTravel();
    return 1;
}


// 474F10
int32_t VisGlobalMap::OnLButtonUp(uint32_t wparam, CPoint pos) {
    return 0;
}


// 471F08
void VisGlobalMap::VMethod8(CRect* rect) {
}


// 47001B (scalar deleting dtor ??_G at 473D70)
VisGlobalMap::~VisGlobalMap() {
    this->FreeBitmaps();
    this->locationHitRects.RemoveAll();
    this->locationPoints.RemoveAll();
    if (this->routeAdjacencyMatrix != nullptr) {
        void** rows = static_cast<void**>(this->routeAdjacencyMatrix);
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            CArray<CPoint>** row = static_cast<CArray<CPoint>**>(rows[i]);
            for (int32_t j = 0; j < this->graphNodePoints.GetSize(); j++) {
                if (row[j] != nullptr) {
                    delete row[j];
                }
            }
        }
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            operator delete(rows[i]);
        }
        operator delete(rows);
    }
    this->routeAdjacencyMatrix = nullptr;
    this->graphNodePoints.RemoveAll();
}


// 46FD7B
VisGlobalMap::VisGlobalMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm)
    : VisScreen(_id, l, t, r, b, btm) {
    this->VMethod26();
}


// 4705C2
void VisGlobalMap::LoadBitmaps() {
    this->FreeBitmaps();
    if (this->umoirMapMode != 0) {
        this->gmap = new CBmp64(A2CLIENT_PATCH_ENABLED ? tangar_locale::gfx_umoir : "main\\graphics\\Global.Map\\Umoir.bmp");
    } else {
        this->gmap = new CBmp64(A2CLIENT_PATCH_ENABLED ? tangar_locale::gfx_gmap : "main\\graphics\\Global.Map\\GMap.bmp");
    }
    g_mousept.Update();
    this->flag1_spr = new CA16("graphics\\Global.Map\\Flag1\\sprites.16a");
    this->flag1_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->flag_spr = new CA16("graphics\\Global.Map\\Flag\\sprites.16a");
    this->flag_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->cross_spr = new CA16("graphics\\Global.Map\\Cross\\sprites.16a");
    this->cross_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->ballmap = new CBmp64("graphics\\Global.Map\\BallMap.bmp");
    g_mousept.Update();
    this->hero_bmp = new CBmp64("graphics\\Global.Map\\Hero.bmp");
    g_mousept.Update();
    this->heroBitmapSize = CSize(this->hero_bmp->GetWidth(0), this->hero_bmp->GetHeight(0));
    this->flg_on_map = new CA16("graphics\\Global.Map\\FlagOnMap\\sprites.16a");
    this->flg_on_map->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->mission_flg = new CA16("graphics\\Global.Map\\MissionFlag\\sprites.16a");
    this->mission_flg->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->yflag_spr = new CA16("graphics\\Global.Map\\YourFlag\\sprites.16a");
    this->yflag_spr->ResetPalette(0x10, 4, 0);
    g_mousept.Update();
    this->scroll1_bmp = new CBmp64("graphics\\Global.Map\\Scroll01.bmp");
    g_mousept.Update();
    this->scroll2_bmp = new CBmp64("graphics\\Global.Map\\Scroll02.bmp");
    g_mousept.Update();
    this->scroll3_bmp = new CBmp64("graphics\\Global.Map\\Scroll03.bmp");
    g_mousept.Update();
    this->scrollp1_bmp = new CBmp64("graphics\\Global.Map\\ScrollP1.bmp");
    g_mousept.Update();
    this->scrollp3_bmp = new CBmp64("graphics\\Global.Map\\ScrollP3.bmp");
    g_mousept.Update();
    this->scrollp2_bmp = new CBmp64("graphics\\Global.Map\\ScrollP2.bmp");
    g_mousept.Update();
}


// 470CD9
void VisGlobalMap::FreeBitmaps() {
    if (this->gmap != nullptr) {
        delete this->gmap;
    }
    this->gmap = nullptr;
    if (this->hero_bmp != nullptr) {
        delete this->hero_bmp;
    }
    this->hero_bmp = nullptr;
    if (this->ballmap != nullptr) {
        delete this->ballmap;
    }
    this->ballmap = nullptr;
    if (this->flag1_spr != nullptr) {
        delete this->flag1_spr;
    }
    this->flag1_spr = nullptr;
    if (this->flag_spr != nullptr) {
        delete this->flag_spr;
    }
    this->flag_spr = nullptr;
    if (this->cross_spr != nullptr) {
        delete this->cross_spr;
    }
    this->cross_spr = nullptr;
    if (this->mission_flg != nullptr) {
        delete this->mission_flg;
    }
    this->mission_flg = nullptr;
    if (this->flg_on_map != nullptr) {
        delete this->flg_on_map;
    }
    this->flg_on_map = nullptr;
    if (this->yflag_spr != nullptr) {
        delete this->yflag_spr;
    }
    this->yflag_spr = nullptr;
    if (this->scroll1_bmp != nullptr) {
        delete this->scroll1_bmp;
    }
    this->scroll1_bmp = nullptr;
    if (this->scroll2_bmp != nullptr) {
        delete this->scroll2_bmp;
    }
    this->scroll2_bmp = nullptr;
    if (this->scroll3_bmp != nullptr) {
        delete this->scroll3_bmp;
    }
    this->scroll3_bmp = nullptr;
    if (this->scrollp1_bmp != nullptr) {
        delete this->scrollp1_bmp;
    }
    this->scrollp1_bmp = nullptr;
    if (this->scrollp3_bmp != nullptr) {
        delete this->scrollp3_bmp;
    }
    this->scrollp3_bmp = nullptr;
    if (this->scrollp2_bmp != nullptr) {
        delete this->scrollp2_bmp;
    }
    this->scrollp2_bmp = nullptr;
}


// 472820
void VisGlobalMap::ComputeTravelRoute(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY) {
    if (CPoint(fromX, fromY) != CPoint(toX, toY)) {
        int32_t from_node = 0;
        int32_t to_node = 0;
        for (int32_t i = 0; i < this->graphNodePoints.GetSize(); i++) {
            CPoint& node = this->graphNodePoints.ElementAt(i);
            if (CPoint(fromX, fromY) != CPoint(node.x, node.y)) {
                if (CPoint(toX, toY) == CPoint(node.x, node.y)) {
                    to_node = i;
                }
            } else {
                from_node = i;
            }
        }
        this->routeNodeIndices.RemoveAll();
        this->routeNodeIndices.routeCost = 0;
        this->bestRouteCost = 2000000000;
        GlobalMapRouteArray* route = new GlobalMapRouteArray();
        route->Add(static_cast<uint16_t>(from_node));
        this->SearchRoute(static_cast<uint16_t>(from_node), static_cast<uint16_t>(to_node), route);
        this->travelRoutePoints.RemoveAll();
        for (int32_t i = 1; i < this->routeNodeIndices.GetSize(); i++) {
            uint16_t prev = this->routeNodeIndices.GetAt(i - 1);
            uint16_t cur = this->routeNodeIndices.GetAt(i);
            CArray<CPoint>*** rows = static_cast<CArray<CPoint>***>(this->routeAdjacencyMatrix);
            this->travelRoutePoints.Append(*rows[prev][cur]);
        }
        delete route;
    } else {
        this->travelRoutePoints.RemoveAll();
        this->travelRoutePoints.Add(CPoint(fromX, fromY));
        this->travelRoutePoints.Add(CPoint(toX, toY));
    }
}


// 472DCA
void VisGlobalMap::OnMapClick() {
    CPoint old_target = this->targetLocationPoint;
    CPoint mouse(g_mousept.GetX(), g_mousept.GetY());
    CPoint screen_pt = this->rect.TopLeft();
    mouse -= CSize(screen_pt.x, screen_pt.y);
    int32_t best_dist = 0x7FFFFFFF;
    ScenarioLocation* best_loc = nullptr;
    for (POSITION pos = ScenarioGetAvailableLocations()->GetHeadPosition(); pos != nullptr;) {
        ScenarioLocation* loc = ScenarioGetAvailableLocations()->GetNext(pos);
        CPoint loc_pt = loc->GetRect().TopLeft();
        int32_t dist = (loc_pt.x - mouse.x) * (loc_pt.x - mouse.x)
            + (loc_pt.y - mouse.y) * (loc_pt.y - mouse.y);
        if (dist < best_dist) {
            best_dist = dist;
            this->targetLocationPoint = loc_pt;
            best_loc = loc;
        }
    }
    if (old_target != this->targetLocationPoint) {
        this->ComputeTravelRoute(this->currentLocationPoint.x, this->currentLocationPoint.y,
            this->targetLocationPoint.x, this->targetLocationPoint.y);
    }
    if (best_loc != nullptr) {
        MissionGetLocName(best_loc->GetKind(), best_loc->GetId(), &this->hoveredLocationTitle);
        this->hoveredLocationTitle = this->hoveredLocationTitle.Left(this->hoveredLocationTitle.GetLength() - 2);
        CString desc;
        MissionGetDescription(best_loc->GetKind(), best_loc->GetId(), &desc);
        this->hoveredLocationLines.Copy(g_font2->StringArrayForRect(CRect(0x46, 0x30, 0x186, 0xC8), desc));
    }
}


// 472D4B
void VisGlobalMap::UpdateHoveredLocation(CPoint pos) {
    CPoint screen_pt = this->rect.TopLeft();
    pos -= CSize(screen_pt.x, screen_pt.y);
    ApplyCursor(g_Cursors[CURSOR_SELECT]);
}


// 470504
void VisGlobalMap::LoadSamples() {
    this->FreeSamples();
    ReloadSfx(&this->snd_scrollup.sample, "SFX\\ScrollUp.wav");
    ReloadSfx(&this->snd_scrolldn.sample, "SFX\\ScrollDn.wav");
    ReloadSfx(&this->snd_point1.sample, "SFX\\Point1.wav");
    ReloadSfx(&this->snd_point2.sample, "SFX\\Point2.wav");
}


// 470571
void VisGlobalMap::FreeSamples() {
    FreeSfxSample(&this->snd_scrollup.sample);
    FreeSfxSample(&this->snd_scrolldn.sample);
    FreeSfxSample(&this->snd_point1.sample);
    FreeSfxSample(&this->snd_point2.sample);
}


// 472684
void VisGlobalMap::FinishTravel() {
    if (this->travelProgress != 0) {
        this->travelProgress = this->travelRoutePoints.GetSize() + 1;
        this->targetCrossAnimationFrame = this->cross_spr->GetFrameCount() + 1;
    }
}


// 472ADA
void VisGlobalMap::SearchRoute(uint16_t from, uint16_t to, GlobalMapRouteArray* route) {
    if (from == to) {
        if (route->routeCost < this->bestRouteCost) {
            this->bestRouteCost = route->routeCost;
            this->routeNodeIndices.Copy(*route);
        }
        return;
    }
    for (int32_t n = 0; n < this->graphNodePoints.GetSize(); n++) {
        if (n == from) {
            continue;
        }
        bool contained = false;
        for (int32_t k = 0; k < route->GetSize(); k++) {
            if (route->GetAt(k) == (uint16_t)n) {
                contained = true;
                break;
            }
        }
        if (contained) {
            continue;
        }
        CArray<CPoint>*** rows = static_cast<CArray<CPoint>***>(this->routeAdjacencyMatrix);
        CArray<CPoint>* segment = rows[from][n];
        if (segment == nullptr) {
            continue;
        }
        if (route->routeCost + segment->GetSize() >= this->bestRouteCost) {
            continue;
        }
        GlobalMapRouteArray* new_route = new GlobalMapRouteArray();
        new_route->Copy(*route);
        new_route->routeCost = route->routeCost;
        new_route->Add(static_cast<uint16_t>(n));
        new_route->routeCost += segment->GetSize();
        this->SearchRoute(static_cast<uint16_t>(n), to, new_route);
        delete new_route;
    }
}




