// Ported from my-a2mgr language.cpp DecideLanguage + path tables.

#include "locale.h"

#ifdef A2CLIENT_PATCH
#include "client_config.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <windows.h>

namespace tangar_locale {

const char* credits = "main\\text\\credits.txt";
const char* globalmap = "main\\text\\globalmap.txt";
const char* help = "main\\text\\help.txt";
const char* main_txt = "main\\text\\main.txt";
const char* heropicture = "main\\text\\heropicture.txt";
const char* stats = "main\\text\\stats.txt";
const char* spells = "main\\text\\spells.txt";
const char* spell = "main\\text\\spell.txt";
const char* dialogs = "main\\text\\dialogs.txt";
const char* unitname = "main\\text\\unitname.txt";
const char* building = "main\\text\\building.txt";
const char* itemname = "main\\text\\itemname.txt";
const char* npcnames = "main\\text\\npcnames.txt";
const char* cutscene = "main\\text\\cutscene.txt";
const char* cutpaths = "main\\text\\cutpaths.txt";
const char* tunes = "main\\text\\tunes.txt";
const char* patch_txt = "patch\\patch.txt";
const char* town = "main\\text\\town.txt";
const char* mission_fmt = "main\\text\\mission%d.txt";
const char* quest = "main\\text\\quest.txt";
const char* docs_fmt = "main\\text\\docs\\%d.txt";

const char* gfx_chrgen_leftup = "main\\graphics\\chrgen\\leftup.bmp";
const char* gfx_logo_dir = "main\\graphics\\logo\\";
const char* gfx_famehall = "main\\graphics\\famehall\\hall.bmp";
const char* gfx_umoir = "main\\graphics\\Global.Map\\Umoir.bmp";
const char* gfx_gmap = "main\\graphics\\Global.Map\\GMap.bmp";
const char* gfx_logo_allods = "main\\graphics\\logo\\allods.bmp";
const char* gfx_menu_mask = "main\\graphics\\MainMenu\\MenuMask.bmp";
const char* gfx_menu = "main\\graphics\\MainMenu\\menu_.bmp";
const char* gfx_button_p_fmt = "main\\graphics\\MainMenu\\button%up.bmp";
const char* gfx_button_fmt = "main\\graphics\\MainMenu\\button%u.bmp";
const char* gfx_text_fmt = "main\\graphics\\MainMenu\\text%u.bmp";

void DecideLanguage() {
#ifdef A2CLIENT_PATCH
    if (!tangar_cfg::z_lang_override) {
        LANGID lid = GetSystemDefaultUILanguage();
        tangar_cfg::z_russian = (lid == 0x419);
    }
    bool is_russian = tangar_cfg::z_russian;
#else
    LANGID lid = GetSystemDefaultUILanguage();
    bool is_russian = (lid == 0x419);
#endif

    const char* dir = is_russian ? "locale\\ru" : "locale\\en";

    char buf[128];
#define SET(var, suffix)                              \
    do {                                              \
        _snprintf(buf, sizeof(buf), "%s%s", dir, suffix); \
        var = _strdup(buf);                           \
    } while (0)

    SET(credits, "\\credits.txt");
    SET(globalmap, "\\globalmap.txt");
    SET(help, "\\help.txt");
    SET(main_txt, "\\main.txt");
    SET(heropicture, "\\heropicture.txt");
    SET(stats, "\\stats.txt");
    SET(spells, "\\spells.txt");
    SET(spell, "\\spell.txt");
    SET(dialogs, "\\dialogs.txt");
    SET(unitname, "\\unitname.txt");
    SET(building, "\\building.txt");
    SET(itemname, "\\itemname.txt");
    SET(npcnames, "\\npcnames.txt");
    SET(cutscene, "\\cutscene.txt");
    SET(cutpaths, "\\cutpaths.txt");
    SET(tunes, "\\tunes.txt");
    SET(patch_txt, "\\patch.txt");
    SET(town, "\\town.txt");
    SET(mission_fmt, "\\mission%d.txt");
    SET(quest, "\\quest.txt");
    SET(docs_fmt, "\\docs\\%d.txt");

    SET(gfx_chrgen_leftup, "\\graphics\\chrgen\\leftup.bmp");
    SET(gfx_logo_dir, "\\graphics\\logo\\");
    SET(gfx_famehall, "\\graphics\\famehall\\hall.bmp");
    SET(gfx_umoir, "\\graphics\\Global.Map\\Umoir.bmp");
    SET(gfx_gmap, "\\graphics\\Global.Map\\GMap.bmp");
    SET(gfx_logo_allods, "\\graphics\\logo\\allods.bmp");
    SET(gfx_menu_mask, "\\graphics\\MainMenu\\MenuMask.bmp");
    SET(gfx_menu, "\\graphics\\MainMenu\\menu_.bmp");
    SET(gfx_button_p_fmt, "\\graphics\\MainMenu\\button%up.bmp");
    SET(gfx_button_fmt, "\\graphics\\MainMenu\\button%u.bmp");
    SET(gfx_text_fmt, "\\graphics\\MainMenu\\text%u.bmp");
#undef SET
}

} // namespace tangar_locale
