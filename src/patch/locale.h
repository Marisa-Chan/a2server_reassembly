#pragma once

#ifdef A2CLIENT_PATCH
#define A2CLIENT_PATCH_ENABLED 1
#else
#define A2CLIENT_PATCH_ENABLED 0
#endif

// Tangar client localization, ported from my-a2mgr language.cpp (DecideLanguage
// + the locale path table). The patched client loads text and graphics from
// locale\<lang>\ instead of the hardcoded main\text\ / main\graphics\ paths;
// the language defaults from the system UI language (Russian -> ru, else en)
// and can be overridden by config.

namespace tangar_locale {

// Resolve the language and fill the path table. Call once at client startup.
void DecideLanguage();

// Text tables (game_app.cpp load block and per-map reads).
extern const char* credits;    // was main\text\credits.txt
extern const char* globalmap;  // was main\text\globalmap.txt
extern const char* help;       // was main\text\help.txt
extern const char* main_txt;       // was main\text\main.txt
extern const char* heropicture;
extern const char* stats;
extern const char* spells;
extern const char* spell;
extern const char* dialogs;
extern const char* unitname;
extern const char* building;
extern const char* itemname;
extern const char* npcnames;
extern const char* cutscene;
extern const char* cutpaths;
extern const char* tunes;
extern const char* patch_txt;   // was patch\patch.txt
extern const char* town;        // was main\text\town.txt
extern const char* mission_fmt; // was main\text\mission%d.txt
extern const char* quest;       // was main\text\quest.txt
extern const char* docs_fmt;    // was main\text\docs\%d.txt

// Graphics overrides.
extern const char* gfx_chrgen_leftup; // was main\graphics\chrgen\leftup.bmp
extern const char* gfx_logo_dir;      // was main\graphics\logo dir
extern const char* gfx_famehall;      // was main\graphics\famehall\hall.bmp
extern const char* gfx_umoir;         // was main\graphics\Global.Map\Umoir.bmp
extern const char* gfx_gmap;          // was main\graphics\Global.Map\GMap.bmp
extern const char* gfx_logo_allods;   // was main\graphics\logo\allods.bmp
extern const char* gfx_menu_mask;     // was main\graphics\MainMenu\MenuMask.bmp
extern const char* gfx_menu;          // was main\graphics\MainMenu\menu_.bmp
extern const char* gfx_button_p_fmt;  // was main\graphics\MainMenu\button%up.bmp
extern const char* gfx_button_fmt;    // was main\graphics\MainMenu\button%u.bmp
extern const char* gfx_text_fmt;      // was main\graphics\MainMenu\text%u.bmp

} // namespace tangar_locale
