// Heroes from the local character files in the oval medallions of the
// menunew.png start screen. See menu_avatars.h.

#include "menu_avatars.h"

#include <afxwin.h>

#include <stdio.h>
#include <timeapi.h>
#include <vector>

#include "../gameobj.h"
#include "../file.h"
#include "../game_app.h"
#include "../gfx.h"
#include "../item.h"
#include "../packet.h"
#include "../player_file.h"
#include "../server.h"
#include "../unit.h"
#include "client_config.h"
#include "image.h"
#include "itempng.h"
#include "stats_level.h"

namespace tangar_menu_avatars {

// One medallion of menunew.png: where its crop sits on the start screen, and
// the affine map x' = a*u + b*v + c, y' = d*u + e*v + f from paper-doll pixels
// to crop pixels (medallions.tsv): a rotation, a uniform scale and a shift, so
// the figure keeps its proportions. The tilt is the painted figure's; the
// figure sits a few pixels lower than that one, centred in the oval.
struct Medallion {
    const char* name;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
};

static const int32_t kMedallionCount = 4;
static const Medallion kMedallions[kMedallionCount] = {
    {"ff", 132, 170, 117, 126, 0.534937f, 0.192008f, -3.936651f, -0.192008f, 0.534937f, 13.123681f},
    {"fm", 290, 134, 112, 123, 0.499290f, 0.169762f, -0.355106f, -0.169762f, 0.499290f, 14.434567f},
    {"mf", 22, 277, 134, 143, 0.535905f, 0.312861f, -12.361430f, -0.312861f, 0.535905f, 28.364222f},
    {"mm", 422, 203, 122, 138, 0.587647f, -0.083951f, 29.047483f, 0.083951f, 0.587647f, -7.447784f},
};

static const char kAssetDir[] = "data\\graphics\\interface\\mainmenu\\medallions\\";
// CUnit::RenderHeroPortrait paints a 160x240 picture.
static const int32_t kPaperDollWidth = 0xa0;
static const int32_t kPaperDollHeight = 0xf0;
// A figure inside a medallion is this bright relative to its paper-doll.
static const float kFigureGain = 0.9f;
// How long a medallion stays lit before another one takes over.
static const uint32_t kHighlightPeriod = 312;

// What one medallion is drawn from.
struct Assets {
    PatchImage* plate = nullptr;     // the backdrop, with the painted figure removed
    PatchImage* frame = nullptr;
    PatchImage* frame_lit = nullptr; // the same frame lit: the highlight
    PatchImage* oval = nullptr;      // where the backdrop and the figure go: the oval, extended under the frame's inner edge
};

static Assets g_assets[kMedallionCount];
// Set once loading succeeded. False: not tried yet, or the files are not on
// disk, in which case the start screen keeps its painted figures.
static bool g_assets_ready = false;
// The finished medallion pictures (straight alpha), rebuilt whenever the menu
// is loaded: for each medallion its normal one, and its lit one if the lit
// frame is on disk.
static uint32_t* g_pictures[kMedallionCount][2] = {};
// The medallion that is lit right now, -1 for none, and when another one takes over.
static int32_t g_highlighted = -1;
static uint32_t g_highlight_deadline = 0;

// A PatchImage pixel is R, G, B, A: 0xAABBGGRR as a little-endian dword.
static uint8_t Red(uint32_t pixel) {
    return (uint8_t)(pixel & 0xFF);
}

static uint8_t Green(uint32_t pixel) {
    return (uint8_t)((pixel >> 8) & 0xFF);
}

static uint8_t Blue(uint32_t pixel) {
    return (uint8_t)((pixel >> 16) & 0xFF);
}

static uint8_t Alpha(uint32_t pixel) {
    return (uint8_t)(pixel >> 24);
}

static bool FileExists(const char* path) {
    FILE* file = nullptr;
    if (fopen_s(&file, path, "rb") == 0 && file != nullptr) {
        fclose(file);
        return true;
    }
    return false;
}

static bool LoadAsset(const Medallion& medallion, const char* kind, PatchImage** slot) {
    char path[MAX_PATH];
    sprintf_s(path, "%s%s_%s.png", kAssetDir, medallion.name, kind);
    *slot = FileExists(path) ? new PatchImage(path) : nullptr;
    return *slot != nullptr;
}

// The backdrop, frames and mask of every medallion. False, and nothing loaded,
// if a backdrop, an unlit frame or a mask is missing; a missing lit frame only
// means that its medallion never lights up.
static bool LoadAssets() {
    bool loaded = true;
    for (int32_t i = 0; i < kMedallionCount && loaded; i++) {
        loaded = LoadAsset(kMedallions[i], "plate", &g_assets[i].plate)
            && LoadAsset(kMedallions[i], "frame", &g_assets[i].frame)
            && LoadAsset(kMedallions[i], "oval", &g_assets[i].oval);
        if (loaded) {
            LoadAsset(kMedallions[i], "frame_lit", &g_assets[i].frame_lit);
        }
    }
    if (!loaded) {
        for (int32_t i = 0; i < kMedallionCount; i++) {
            delete g_assets[i].plate;
            delete g_assets[i].frame;
            delete g_assets[i].frame_lit;
            delete g_assets[i].oval;
            g_assets[i] = Assets();
        }
    }
    return loaded;
}

static void ListCharacterFiles(std::vector<CString>& files) {
    WIN32_FIND_DATAA found;
    const HANDLE search = FindFirstFileA("*.a2c", &found);
    if (search == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            files.push_back(found.cFileName);
        }
    } while (FindNextFileA(search, &found) != 0);
    FindClose(search);
}

static void Shuffle(std::vector<CString>& files) {
    for (size_t i = files.size(); i > 1; i--) {
        const size_t j = (size_t)(rand() % (int32_t)i);
        const CString file = files[j];
        files[j] = files[i - 1];
        files[i - 1] = file;
    }
}

// ParsePlayerFile reads each section into a fixed buffer without looking at
// how large the section claims to be. That is fine for a file the player picks
// in the character select; the start screen picks files on its own, among them
// ones that screen never opens (other servers' characters, damaged ones), so
// the section headers are checked first: every section must be whole, of a
// known kind, and no larger than the buffer it is read into. The limits leave
// ample room over real files (a full inventory is a few hundred bytes).
// `equipment_size` receives the size of the equipment section, 0 if there is none.
static bool SectionsLookSound(CFile* file, uint32_t* equipment_size) {
    *equipment_size = 0;
    uint32_t magic = 0;
    file->Read(&magic, sizeof(magic));
    if (magic != player_file::A2C_MAGIC) {
        return false;
    }
    while (file->GetPosition() < file->GetLength()) {
        PlayerFileSectionHeader header;
        if (file->Read(&header, sizeof(header)) != sizeof(header)) {
            return false;
        }
        uint32_t smallest = 0;
        uint32_t largest = 0;
        switch (header.tag) {
        case player_file::SEC_BASIC_INFO:
            smallest = sizeof(FileSectionBasicInfo);
            largest = sizeof(FileSectionBasicInfo);
            break;
        case player_file::SEC_STATS:
            largest = 0x80; // read field by field; 64 bytes at most in practice
            break;
        case player_file::SEC_KILL_STATS:
        case player_file::SEC_UNKNOWN_40A:
            largest = 0x400; // into buffers of 0xA00 bytes
            break;
        case player_file::SEC_EQUIPMENT:
        case player_file::SEC_INVENTORY:
            largest = 0x4000; // into a buffer of 0x8000 bytes
            break;
        default:
            return false;
        }
        if (header.size < smallest || header.size > largest || header.size > file->GetLength() - file->GetPosition()) {
            return false;
        }
        if (header.tag == player_file::SEC_EQUIPMENT) {
            *equipment_size = header.size;
        }
        file->Seek(header.size, CFile::current);
    }
    return true;
}

// Whether the game can load this sprite: a PNG twin under data\ (the Tangar
// replacement), or the .256 itself. CUnit::RenderHeroPortrait stops the game
// when a sprite is missing, so everything it loads is checked first.
static bool SpriteLoadable(const CString& path) {
    if (TangarLoadItemSprite(path) != nullptr) {
        return true;
    }
    ResArchiveFile file;
    return file.Open(path, CFile::modeRead) != FALSE;
}

// Whether every sprite CUnit::RenderHeroPortrait loads for the item worn in
// equipment slot `index` is there: its main one, and for some slots a second.
static bool HasArt(TokenEntry* token, int32_t index, const CString& sprite_prefix, bool mage) {
    const CString name = token->GetSpriteName();
    if (!SpriteLoadable(sprite_prefix + "primary\\" + name + ".256")) {
        return false;
    }
    const bool second = index == 3 || index == 8 || index == 9 || (index == 7 && mage);
    return !second || SpriteLoadable(sprite_prefix + "secondary\\" + name + ".256");
}

// The hero of one character file, painted into `picture` by
// CUnit::RenderHeroPortrait, on a private unit built straight from the file:
// its class, its look and the items it wears. Nothing of the game state is
// touched, and no file is loaded that has not been found first.
static bool BuildPaperDoll(const char* path, CBmp64* picture) {
    CFile file;
    if (!file.Open(path, CFile::modeRead | CFile::shareDenyNone)) {
        return false;
    }
    uint32_t equipment_size = 0;
    if (!SectionsLookSound(&file, &equipment_size)) {
        return false;
    }
    file.SeekToBegin();

    FileSectionBasicInfo* basic = nullptr;
    FileSectionStats* stats = nullptr;
    uint8_t* kill_stats = nullptr;
    PacketUnitStateVec* equipped = nullptr;
    PacketUnitStateVec* bag = nullptr;
    uint8_t* section_40a = nullptr;
    uint32_t size_40a = 0;
    // Parsing announces to the stats patch that an existing character is being
    // loaded; the start screen must not.
    const bool creating_new = tangar_stats::creating_new_character;
    const int32_t parsed = ParsePlayerFile(&file, &basic, &stats, &kill_stats, &equipped, &bag, &section_40a, &size_40a);
    tangar_stats::creating_new_character = creating_new;
    if (parsed == 0 || basic == nullptr) {
        return false;
    }

    const bool mage = basic->character_class.is_mage != 0;
    const bool female = basic->character_class.is_female != 0;
    CString sprite_prefix = "graphics\\equipment\\";
    sprite_prefix += mage ? (female ? "fmage" : "mmage") : (female ? "ffighter" : "mfighter");
    sprite_prefix += "\\";

    CUnit* unit = new CUnit();
    // ApplyFace takes the class from the unit type and the look from the face.
    unit->typeId = static_cast<uint16_t>(unit_type::hero_fighter_male + 2 * mage + female);
    unit->ApplyFace(basic->picture);

    CString body;
    body.Format("%s%d.256", sprite_prefix.GetString(), basic->picture);
    bool shown = SpriteLoadable(body);

    // The equipment stream holds twelve items in slot order, each as
    // TokenEntry reads it. Only the packet's own data is the stream: past it
    // lie bytes left over from parsing another file. An item the game does
    // not know stops that constructor after its id, so from there the stream
    // cannot be followed.
    if (shown && equipped != nullptr) {
        uint8_t* data = equipped->data;
        // The stream is as long as the packet header says, and never longer than the section.
        uint32_t data_size = equipment_size >= 9 ? equipment_size - 9 : 0;
        if (equipped->data_size < data_size) {
            data_size = equipped->data_size;
        }
        const uint8_t* end = data + data_size;
        for (int32_t index = 0; index < 12 && data + 3 <= end; index++) {
            const ItemId id = *(uint16_t*)data;
            ItemNamePktEntry* known = nullptr;
            if (id != ItemId::NONE && id < ItemId::GOLD_PSEUDO_MIN && g_ItemNamePktMap.Lookup(id, *(void**)&known) == 0) {
                break;
            }
            TokenEntry* token = new TokenEntry(&data, 0);
            // The weapon's id selects a line of heropicture.txt, which has no line 0 - 1 and none past its end.
            const bool weapon_in_range = index != 0 || (token->GetId() != 0 && token->GetId() <= txt_heropicture.GetCount());
            if (token->item_id != ItemId::NONE && weapon_in_range && HasArt(token, index, sprite_prefix, mage)) {
                unit->equipmentTokens[index] = token;
            } else {
                delete token; // empty, or without its art: the hero is shown without it
            }
        }
    }

    // The portrait is painted through the game's draw target, clipped to the
    // clip rectangle; both are restored by the caller.
    if (shown) {
        SetClipRect(0, 0, kPaperDollWidth, kPaperDollHeight);
        unit->RenderHeroPortrait(nullptr, picture, nullptr);
    }
    delete unit;
    return shown;
}

// The picture is RGB565; its 5 and 6 bit channels are spread to 8 bits.
static void ToRgb(uint16_t pixel, float* colour) {
    const int32_t red = (pixel >> 11) & 0x1F;
    const int32_t green = (pixel >> 5) & 0x3F;
    const int32_t blue = pixel & 0x1F;
    colour[0] = (float)((red << 3) | (red >> 2));
    colour[1] = (float)((green << 2) | (green >> 4));
    colour[2] = (float)((blue << 3) | (blue >> 2));
}

// One bilinear sample of the paper-doll at (u, v), counted from its top-left
// corner: how much the figure covers the pixel (0..1), and the figure's colour.
// Black is where nothing was painted.
static float SamplePaperDoll(const uint16_t* paper_doll, float u, float v, float* colour) {
    if (u < 0 || v < 0) {
        return 0.0f;
    }
    const int32_t x = (int32_t)u;
    const int32_t y = (int32_t)v;
    if (x >= kPaperDollWidth - 1 || y >= kPaperDollHeight - 1) {
        return 0.0f;
    }
    const float fx = u - x;
    const float fy = v - y;
    const uint16_t* row = paper_doll + y * kPaperDollWidth + x;
    // top-left, top-right, bottom-left, bottom-right
    const uint16_t corners[4] = { row[0], row[1], row[kPaperDollWidth], row[kPaperDollWidth + 1] };
    const float weights[4] = { (1.0f - fx) * (1.0f - fy), fx * (1.0f - fy), (1.0f - fx) * fy, fx * fy };
    // Only painted corners count, so that empty ones do not darken the fringe.
    float coverage = 0.0f;
    float sum[3] = { 0.0f, 0.0f, 0.0f };
    for (int32_t i = 0; i < 4; i++) {
        if (corners[i] == 0) {
            continue;
        }
        float corner[3];
        ToRgb(corners[i], corner);
        coverage += weights[i];
        for (int32_t c = 0; c < 3; c++) {
            sum[c] += weights[i] * corner[c];
        }
    }
    if (coverage < 1.0f / 510.0f) {
        return 0.0f;
    }
    for (int32_t c = 0; c < 3; c++) {
        colour[c] = sum[c] / coverage;
    }
    return coverage;
}

// The finished picture of one medallion with a hero in it (straight alpha):
// the backdrop, the hero's paper-doll seen through the medallion's map and
// darkened, and a frame on top: the unlit one, or the lit one that highlights
// the medallion. Only what replaces the painted figure is opaque: the oval
// behind the frame's inner edge, and the frame's solid pixels. The unlit
// frame's half-covered outer edge is left to menunew.png, which has it
// already; the lit one brings its own along, to be blended in when drawn.
static uint32_t* BuildPicture(const Medallion& medallion, const Assets& assets, const uint16_t* paper_doll, const uint32_t* frame, bool lit) {
    const int32_t width = medallion.width;
    const int32_t height = medallion.height;
    const uint32_t* plate = assets.plate->GetPixels();
    const uint32_t* oval = assets.oval->GetPixels();
    uint32_t* picture = new uint32_t[width * height];
    // The inverse of the medallion's map.
    const float det = medallion.a * medallion.e - medallion.b * medallion.d;
    for (int32_t y = 0; y < height; y++) {
        const float dy = ((float)y - height * 0.5f) / (height * 0.5f);
        for (int32_t x = 0; x < width; x++) {
            const int32_t index = y * width + x;
            const bool in_oval = Alpha(oval[index]) != 0;
            const uint8_t frame_alpha = Alpha(frame[index]);
            if (!in_oval && frame_alpha == 0) {
                picture[index] = 0;
                continue;
            }
            if (!in_oval && frame_alpha != 0xFF) {
                // the half-covered outer edge of the frame
                picture[index] = lit ? (frame[index] & 0x00FFFFFF) | ((uint32_t)frame_alpha << 24) : 0;
                continue;
            }

            // the backdrop; black where the oval runs under the frame
            float base[3] = { 0.0f, 0.0f, 0.0f };
            if (Alpha(plate[index]) != 0) {
                base[0] = (float)Red(plate[index]);
                base[1] = (float)Green(plate[index]);
                base[2] = (float)Blue(plate[index]);
            }

            // the figure, a little darker toward the rim of the medallion
            const float dx = ((float)x - width * 0.5f) / (width * 0.5f);
            const float map_x = (float)x - medallion.c;
            const float map_y = (float)y - medallion.f;
            const float u = (medallion.e * map_x - medallion.b * map_y) / det;
            const float v = (medallion.a * map_y - medallion.d * map_x) / det;
            float figure[3];
            const float coverage = SamplePaperDoll(paper_doll, u, v, figure);
            if (coverage > 0.0f) {
                const float gain = kFigureGain * (1.0f - 0.125f * (dx * dx + dy * dy));
                for (int32_t c = 0; c < 3; c++) {
                    base[c] = coverage * gain * figure[c] + (1.0f - coverage) * base[c];
                }
            }

            const float cover = frame_alpha / 255.0f;
            const float channels[3] = { (float)Red(frame[index]), (float)Green(frame[index]), (float)Blue(frame[index]) };
            uint32_t packed = 0xFF000000;
            for (int32_t c = 0; c < 3; c++) {
                const uint8_t value = (uint8_t)(cover * channels[c] + (1.0f - cover) * base[c] + 0.5f);
                packed |= (uint32_t)value << (8 * c);
            }
            picture[index] = packed;
        }
    }
    return picture;
}

// The medallion to light up next: one with a hero, other than the one lit
// right now, so that the highlight moves on as long as two medallions have heroes.
static int32_t NextHighlighted() {
    int32_t candidates[kMedallionCount];
    int32_t count = 0;
    for (int32_t i = 0; i < kMedallionCount; i++) {
        if (g_pictures[i][1] != nullptr && i != g_highlighted) {
            candidates[count++] = i;
        }
    }
    return count == 0 ? -1 : candidates[rand() % count];
}

void Load() {
    g_highlighted = -1;
    for (int32_t i = 0; i < kMedallionCount; i++) {
        for (int32_t state = 0; state < 2; state++) {
            delete[] g_pictures[i][state];
            g_pictures[i][state] = nullptr;
        }
    }
    if (!tangar_cfg::menu_medallions) {
        return; // switched off in allods2.cfg: the start screen keeps its painted figures
    }
    if (!g_assets_ready && !LoadAssets()) {
        return;
    }
    g_assets_ready = true;

    std::vector<CString> files;
    ListCharacterFiles(files);
    Shuffle(files);

    // Painting goes through the game's draw target and clip rectangle, which
    // may be whatever the last drawn control left behind: both are put back
    // once every hero is painted, whatever became of the files.
    CRect saved_clip;
    GetClipRect(&saved_clip);
    const DDSURFACEDESC saved_target = g_selDrawBitmap;

    int32_t slot = 0;
    for (size_t i = 0; i < files.size() && slot < kMedallionCount; i++) {
        CBmp64 paper_doll(kPaperDollWidth, kPaperDollHeight);
        bool built = false;
        try {
            built = BuildPaperDoll(files[i], &paper_doll);
        } catch (CException* e) {
            e->Delete(); // however damaged a file is, the start screen must not suffer for it
        }
        if (built) {
            const uint16_t* paper_doll_pixels = (const uint16_t*)paper_doll.GetData();
            g_pictures[slot][0] = BuildPicture(kMedallions[slot], g_assets[slot], paper_doll_pixels, g_assets[slot].frame->GetPixels(), false);
            if (g_assets[slot].frame_lit != nullptr) {
                g_pictures[slot][1] = BuildPicture(kMedallions[slot], g_assets[slot], paper_doll_pixels, g_assets[slot].frame_lit->GetPixels(), true);
            }
            slot++;
        }
    }
    SetClipRect(saved_clip);
    g_selDrawBitmap = saved_target;
    // The first highlight comes one period after the menu is shown.
    g_highlight_deadline = timeGetTime() + kHighlightPeriod;
}

void Draw(int32_t left, int32_t top) {
    if (g_IsServer != 0 || g_selDrawBitmap.lpSurface == nullptr) {
        return;
    }
    if ((int32_t)(timeGetTime() - g_highlight_deadline) >= 0) {
        g_highlighted = NextHighlighted();
        g_highlight_deadline = timeGetTime() + kHighlightPeriod;
    }
    uint16_t* surface = (uint16_t*)g_selDrawBitmap.lpSurface;
    const int32_t pitch = g_selDrawBitmap.lPitch / 2;
    for (int32_t i = 0; i < kMedallionCount; i++) {
        const bool lit = i == g_highlighted && g_pictures[i][1] != nullptr;
        const uint32_t* picture = g_pictures[i][lit ? 1 : 0];
        if (picture == nullptr) {
            continue; // no hero for this medallion: it keeps its painted figure
        }
        const Medallion& medallion = kMedallions[i];
        for (int32_t y = 0; y < medallion.height; y++) {
            const int32_t row = top + medallion.y + y;
            if (row < g_ScreenSize.top || row > g_ScreenSize.bottom) {
                continue;
            }
            for (int32_t x = 0; x < medallion.width; x++) {
                const int32_t column = left + medallion.x + x;
                if (column < g_ScreenSize.left || column >= g_ScreenSize.right || column >= pitch) {
                    continue;
                }
                const uint32_t pixel = picture[y * medallion.width + x];
                const int32_t alpha = Alpha(pixel);
                if (alpha == 0) {
                    continue;
                }
                const int32_t red = Red(pixel) >> 3;
                const int32_t green = Green(pixel) >> 2;
                const int32_t blue = Blue(pixel) >> 3;
                uint16_t* destination = &surface[row * pitch + column];
                if (alpha == 0xFF) {
                    *destination = (uint16_t)((red << 11) | (green << 5) | blue);
                } else {
                    // the lit frame's half-covered outer edge, over what is already there
                    const uint16_t under = *destination;
                    const int32_t rest = 255 - alpha;
                    const int32_t mixed_red = (red * alpha + ((under >> 11) & 0x1F) * rest + 127) / 255;
                    const int32_t mixed_green = (green * alpha + ((under >> 5) & 0x3F) * rest + 127) / 255;
                    const int32_t mixed_blue = (blue * alpha + (under & 0x1F) * rest + 127) / 255;
                    *destination = (uint16_t)((mixed_red << 11) | (mixed_green << 5) | mixed_blue);
                }
            }
        }
    }
}

} // namespace tangar_menu_avatars
