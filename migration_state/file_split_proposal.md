# Source File Split Proposal

Date: 2026-11-08
Status: EXECUTED 2026-11-08 (commits d86535a4..afdefb27; execution notes at the bottom)
Checker: `python tool/file_layout_analysis.py` (prints mismatches/splits/strays; run before and
after every batch — its three report sections should shrink to empty, modulo documented exceptions)

---

## 1. Where we are

55 stems in `src/`, each nominally a family. The layout is *mostly* right — world.cpp,
net.cpp, gfx.cpp, gameobj.cpp, quest*.cpp, item.cpp etc. are already cohesive family files.
The problems are concentrated in three places:

### Problem A: `visual.h` / `visual.cpp` are megafiles

- `visual.h`: 3,847 lines, ~113 full class definitions + ~40 forward decls + 123 ASSERT macros.
  It contains the widget toolkit, *every* screen class in the game, and (misplaced) the
  in-game screen's data types.
- `visual.cpp`: 26,890 lines, 115 classes — the largest source file by 3.2x. 60 commits
  touched it in the last two weeks alone; every parallel task lands in the same file.

### Problem B: declaration/definition mismatches (8 real ones)

| Class | Declared in | Defined in (methods) | Note |
|---|---|---|---|
| `BigStruct2` | visual.h | ingame.cpp (131) | the in-game screen; ~6,150 lines |
| `TakeDamage` | visual.h | ingame.cpp (13) | combat resolution logic, not UI; ~1,395 lines |
| `GM_a28` | visual.h | ingame.cpp (7) | ~200 lines |
| `CSound` | sound.h | visual.cpp (128), sound.cpp (1) | ~1,413 lines of audio code in the UI megafile |
| `Resources` | resource.h | game_app.cpp (14), resource.cpp (10), table.cpp (1) | split 3 ways |
| `HatBrowserProgress` | net.h | visual.cpp (4) | |
| `GameSettings` | game_app.h | main_window.cpp (3) | `g_settings` is a hub global; decl stays |
| `ScenarioLocation` | game_app.h | scenario.cpp (2) | rest of scenario.cpp is free functions |

Plus stray one-off definitions split from their class's home file:

- `Building`: outpost.cpp (1), shop.cpp (1) — home is building.cpp
- `CVisualObject`: ingame.cpp (2) — home is visual.cpp
- `Token`: effect.cpp, item.cpp, sack.cpp, building.cpp, unit.cpp, spell_effect.cpp,
  virtual_caster.cpp (1 each) — home is token.cpp
- `ServerControlDialog`: game.cpp (1) — home is main_window.cpp; game.cpp is a 70-line stub
  that should disappear
- `SfxBank`: declared gameobj.h, defined sound.cpp (2)
- Table-row structs declared in the wrong headers: `MagicItem`, `WorldEquip`
  (shop_assortment.h → defs in table.cpp), `BuildingInfo` (building.h → defs in table.cpp),
  `PacketUnitStateVec` (item.h → defs in packet.cpp)

### Problem C: fame family is scattered

`CFameHall` (12 methods) + `Fame2` (9) live in main_window.{h,cpp}; `VisFameHall` /
`VisFameDocument` live in visual.{h,cpp}. One family, two unrelated stems.

---

## 2. Constraints (what the split must not break)

1. **Flat `src/` only.** All tooling globs `src/*.h` / `src/*.cpp`:
   `vis_recheck.py` (scan + verify_queue.find_definitions), `client_refs.py`,
   `ghidra_backport.py`, `ghidra_sync_structs.py`. No subdirectories unless those globs
   are updated first.
2. **Address comments are immutable payload.** `// 54800E` and `(client: <va>)` markers
   travel with their declarations/definitions verbatim. The recheck logs are keyed by
   address, not by file, so history stays valid across moves.
3. **`client_refs.py status` must show the same declaration count before and after each
   batch** — it re-parses every header live, so this is the mechanical proof that nothing
   was dropped or duplicated.
4. **`main_window.cpp` is not split.** The mainwindow recheck family hardcodes it
   (`MAINWINDOW_CPP`). Same logic says: when the net recheck family (AGENTS.md) gets
   implemented, its scanner must glob `src/*.cpp` instead of hardcoding net.cpp.
5. **Every batch ends with `tool/build.ps1` green** (it builds both rea2s and rea2c).
6. **One commit per family move** (user's git-hygiene rule; squash fixups into the move
   commit). Use `git mv` + pure content moves first so `git log --follow` survives.

---

## 3. Target layout rules

- **R1: stem = family.** A header and its same-stem cpp own a class family: declarations
  and definitions in the same pair.
- **R2: definitions follow declarations.** A stray def moves to the declaring header's
  stem cpp.
- **R3: declarations move only when the header is semantically wrong** (e.g. TakeDamage —
  game logic — has no business in visual.h). When the declaring header is a legitimate hub
  (`g_settings` in game_app.h), the defs move instead (R2).
- **R4: `visual.h` stays as an umbrella shim** so the 3 direct includers
  (main_window.h, ingame.cpp, visual.cpp) and the 11 cpps that pull it transitively via
  main_window.h keep compiling untouched. New code can include family headers directly.
- **R5: no file over ~6k lines** where avoidable. (`gameplay.cpp` lands just over — see
  below — because BigStruct2 alone is 6k+; splitting a single class across files is worse
  than a large file.)

---

## 4. The visual.* split — family assignment

Phase 1 renames the pair, phase 2+ extracts families one commit at a time.

### Phase 1 — rename + umbrella (one commit)

```
git mv src/visual.cpp src/vis_core.cpp        # content unchanged
git mv src/visual.h  src/vis_core.h           # content unchanged
# new src/visual.h = umbrella shim:
#   #include "vis_core.h" + every vis_*.h / gameplay.h below
#   (+ keep the old transitive deps: sound.h, gameobj.h, txtfile.h)
```

`vis_core.h`/`vis_core.cpp` keep (~5,000 lines): `CVisualObject`, `VisLabel`, `VisButton`,
`VisScrollBar`, `VisListBox`, `VisTextBox`, `VisRadioBase/Type1/Type2`, `VisBitmap`,
`VisComboBox{,Text,List,Button}`, `VisMultiText`, `HintedListBox`, `VisScreen`, `VisWindow`,
`VisMenuWnd`, `VisRoleKeyScreen`, `VisMessageBox{,WithList}`, `MenuButton`, the `VisMsg`
enum, shared child-window IDs, and the general forward-decl block.

### Phase 2 — family extraction (one commit per stem)

Each new `vis_X.cpp` starts with a copy of vis_core.cpp's include block (pruned later);
each `vis_X.h` includes `vis_core.h` plus only what it needs. Classes are moved with their
ASSERT blocks. Estimated sizes from current line measurements:

| New stem | Classes moved (decl + defs) | ~Lines |
|---|---|---|
| `vis_char` | VisCharSelect, VisCharSellectButtons/List/Stats, VisCharGen, VisCharGenStats/FullStats/Action/Skills, VisCharInfo | 3,000 |
| `vis_inventory` | VisInvBase, VisInvType1, VisInvExtBase, VisInvExtType1/2/3, ExtType1Kaarg/Druid, VisSpellBook | 2,800 |
| `vis_shop` | VisShop{,Druid,Kaarg}, VisShopButtons, VisShopCompass{,Druid,Kaarg} | 2,500 |
| `vis_tavern` | VisTav{,Druid,Kaarg}, VisTavScene{,Druid,Kaarg}, VisTavSceneAnim, VisTavLeftPanel, VisTavRightPanel | 2,200 |
| `vis_town` | VisTown{,Druid,Kaarg}, TownMenuListDialogVisualObject | 1,800 |
| `vis_net` | VisNetDlg, VisNetTcpIp, VisNetSerialSettings, VisNetPhoneBook, VisListBoxPhoneBook, VisNetMapList, VisNetChatList, VisNetPlayerList, VisNetChatTextBox, VisNetMapSelection, NetMapInfo, VisConnectionDlg, VisServerScreen, VisServerScreenRadio | 1,900 |
| `vis_menu` | VisMainMenu, VisLogoWnd, VisCredits, VisStartGame{,TextBox}, SaveGameWindow, LoadGameWindow, VisButtonSaveGame, GameOptionsWindow, SoundPreferencesDialogVisualObject, IngameMenu, EndGameMenu, ExitGameMenu, VisTipsDialog, VisQuestStatus, QuestObjectivesHeaderDialogVisualObject, Vis2Action, VisMissionFailed, VisCutScenesDlg, VisDiplomacy, VisListBoxDiplomacy, VisDropGold | 3,000 |
| `vis_fame` | VisFameHall, VisFameDocument **+ CFameHall, Fame1, Fame2 from main_window.{h,cpp}** | 1,100 |
| `vis_hat` | VisHatServerListDlg, VisHatBrowserDlg, VisHatBrowserList, HatPlayerListWindow, HatPlayerScoreList, VisHatSession **+ HatBrowserProgress decl from net.h** | 850 |
| `vis_worldmap` | VisGlobalMap, GMapThing | 750 |
| `take_damage` | TakeDamage (from visual.h + ingame.cpp) — pure combat-resolution logic | 1,400 |
| `gameplay` | **BigStruct2 + GM_a28** (from visual.h + ingame.cpp) **+ HUD children from visual.\***: VisRightPanel, VisMiniMap, VisOrderToolbar, VisSideStatus, Vis1200, Vis1200obj | 8,000 |

Bin notes (binding rules are R1–R5; individual bins may be adjusted while executing if a
compile dependency says otherwise):
- `VisSpellBook` is a slot-grid panel like the inventory — bins with vis_inventory.
- `VisDropGold` / tips / quest status / diplomacy dialogs bin with vis_menu (modal screens);
  move to gameplay if their includes turn out cleaner there.
- `Vis1200`/`Vis1200obj` hold `BigStruct2*` back-pointers and only exist inside the
  in-game screen — they bin with gameplay. Pointer-only references are satisfied by
  forward declarations (the existing `class BigStruct2;` pattern), so no header cycle:
  gameplay.h is included by the cpps, not by child headers.

### End state of the old files

- `visual.h` — ~30-line umbrella (includes vis_core.h + all stems above).
- `vis_core.{h,cpp}` — widget toolkit, ~5,000 lines.
- `ingame.cpp` shrinks 8,345 → ~600 lines (gfx-info wrappers matching ingame.h:
  UnitVFXUnfo, GfxFile, GfxObject, UnitGfxFile, StructureInfo, ProjectileInfo).
- `main_window.cpp` shrinks ~6,710 → ~6,300 (fame family leaves).

---

## 5. Mismatch fixes (independent of Phase 2; do first, they're small)

One commit each, in this order:

1. **CSound → sound.cpp.** Move 128 method defs from visual.cpp (pre-rename) to sound.cpp.
   Fixes the single worst decl/def mismatch with zero declaration moves.
2. **Resources → resource.cpp.** Move 14 defs from game_app.cpp + 1 from table.cpp.
3. **Stray defs home.** Building (outpost.cpp, shop.cpp → building.cpp); CVisualObject
   (ingame.cpp → visual.cpp); Token (7 files → token.cpp); ServerControlDialog
   (game.cpp → main_window.cpp) and delete game.cpp; GameSettings defs
   (main_window.cpp → game_app.cpp, decl stays — R3).
4. **Wrong-header declarations.** ScenarioLocation (game_app.h → map_stuff.h, next to
   `Scenario`; defs stay in scenario.cpp which anchors to map_stuff.h);
   SfxBank (gameobj.h → sound.h); MagicItem + WorldEquip (shop_assortment.h → table.h);
   BuildingInfo (building.h → table.h); PacketUnitStateVec (item.h → packet.h);
   HatBrowserProgress (net.h → rides with vis_hat in Phase 2).
5. **Fame consolidation** can ride as its own commit before or during Phase 2
   (CFameHall + Fame1/Fame2 out of main_window.*).

### Documented exceptions (leave alone deliberately)

- **CGameSession**: declared main_window.h, 16 defs in main_window.cpp, but
  `WritePlayerFile` (~500 lines, the player-file format writer) sits in player_file.cpp
  next to the file-format structs — semantically right where it is. Optional follow-up if
  it keeps itching: new stem `game_session.{h,cpp}`.
- **HatSettings** methods in main_window.cpp (97 lines) — small, net-declared; revisit
  with the net recheck family.
- `MFC/mfc_templ_impl.h`, `dplay.h`, `dplobby.h`, `assert_offset.h`, `2darray.h`,
  `bitset.h`, `constants.h` — infrastructure headers, correct as-is.

### Left as-is (already cohesive families)

`world.cpp` (World, 228 methods — one class, one stem), `map_stuff.cpp` (MapStuff +
Scenario), `server.cpp`, `gfx.cpp`, `gameobj.cpp`, `unit.cpp`, `net.cpp` (future
net-recheck family; if it ever needs splitting, fix the scanner glob first),
`item.cpp`, `inn.cpp`, `quest*.cpp`, `table.cpp`, `resource.cpp`, `sound.cpp` (after
CSound lands), `shelf.cpp`, `packet.cpp`, `effect.cpp`/`spell*.cpp`, `alm.cpp`,
`mouse.cpp`, `eye.cpp`, and the small stems.

---

## 6. Execution checklist (per commit)

1. `git mv` / move class blocks (decls with their ASSERTs, defs with their `// addr` and
   `(client: ...)` comments) — pure moves, no edits in the same commit where possible.
2. Update `src/CMakeLists.txt` `A2_SOURCES` (new cpps; remove game.cpp when deleted).
3. `tool/build.ps1` — green for both targets.
4. `python tool/client_refs.py status` — declaration/pair counts unchanged.
5. `python tool/file_layout_analysis.py` — one fewer mismatch/stray per section.
6. Commit message: `Refactor: split <family> into <stem>.{h,cpp}` (or
   `Refactor: move <Class> defs to <file>`); squash fixups into it.

Suggested order overall: §5 mismatches first (small, fast wins, ~5 commits), then Phase 1
(umbrella rename), then Phase 2 families biggest-first: gameplay → vis_menu → vis_char →
vis_inventory → vis_shop → vis_tavern → vis_town → vis_net → vis_fame → vis_hat →
vis_worldmap. Biggest-first maximizes merge-conflict relief early; gameplay first also
empties ingame.cpp so it stops being a second megafile.

## 7. Risks

- **Transitive include loss**: visual.h today drags in sound.h/gameobj.h/txtfile.h and
  every screen; the umbrella shim preserves exactly that, so nothing changes for consumers.
  Risk appears only if someone later includes a family header directly with a hidden
  transitive dependency — caught at compile time, fix by adding the needed include.
- **Static-init order**: no dynamically-initialized globals live at file scope in the
  affected regions (verified by grep; globals are POD `extern` + init functions), so
  moving code between TUs does not change initialization order in practice.
- **Tooling**: everything address-keyed and glob-based; `find_arg_names.py` has a
  hardcoded file list (update when used); `asm_find_vtables.py` mentions visual.h but is a
  one-shot historical tool.
- **Rebase pain during execution**: land batches quickly; each commit is independently
  green, so in-flight work can rebase onto any intermediate commit.

---

## 8. Execution record (2026-11-08)

Commits, in order: d86535a4 (proposal+checker) → 5010a710 (stray defs) → ec198ebd
(ScenarioLocation decl) → 42fc7a02 (Phase 1 vis_core rename + umbrella) → f6dffd5b
(gameplay + take_damage) → ffc3aa92 (vis_menu) → e6b91536 (vis_char) → f0ea89e2
(vis_inventory) → b0c06847 (vis_shop) → 2522eebb (vis_tavern + SortTokenEntries home)
→ c91cf974 (vis_town) → 3fe98eb1 (vis_net) → 31efbdfe (vis_worldmap) → 7f4a7c7e
(vis_fame incl. CFameHall/Fame1/Fame2 out of main_window) → 02397011 (vis_hat +
HatBrowserProgress out of net.h). (A one-shot split_family.py helper drove the
extractions; deliberately not committed.)

Deviations from the plan as written:

- §5 items 1–2 (CSound, Resources) and the Building/CVisualObject/Token/SfxBank/
  MagicItem/WorldEquip/BuildingInfo/PacketUnitStateVec rows were **checker false
  positives**: the old `def_re` counted qualified call sites (`CSound::Play(x);`) and
  forward declarations as definitions. The checker now verifies a definition's
  signature terminates in `{` before any `;` and ignores `class X;` lines. After the
  fix only 6 real mismatches remained; all resolved except ScenarioLocation
  (documented exception — Read/Write defs stay in scenario.cpp by design).
- gameplay + take_damage landed in one commit (their ingame.cpp def runs interleave).
- vis_core.cpp's file-scope statics moved with their families: startgame_*,
  blink_*, tavscene_*/druid/kaarg + kaarg_frame_script, lizard_seq*, netdlg_* +
  EnumSessionsPump/ExtractFirstWord, g_Spellbook* tables, s_MainMenu*Rects,
  g_EmptyStr{Load,Retry,Confirm,Cancel,SrvScreenText,MapTooltip,NetChatText},
  gmap_repaint_*, SortTokenEntries+CompareShopTokenEntries (→ vis_shop.cpp),
  g_ActionDirDx/Dy + g_SpellOffensive + g_CUnitStatic + clr_log_* (→ gameplay.cpp).
  `g_HintBuf` stays in vis_core.cpp with a new `extern` in vis_core.h (shared
  GetHint buffer used by vis_core and vis_inventory).
- VisHatSession's definitions are wrapped in `#ifdef A2CLIENT` inside vis_hat.cpp
  (the original wrapper was a file-level block the extractor split); its declaration
  stays unconditional, matching the old header.
- vis_fame structs moved out of main_window.h as one block (Fame1/Fame2/CFameHall);
  main_window.h still sees them via the umbrella (it includes visual.h).
- The vis_tavern commit (2522eebb) was amended once: it initially staged a stale
  vis_core.h that would have duplicated the VisTav family across vis_core.h and
  vis_tavern.h; the amended commit builds green.

End state: visual.h = 28-line umbrella; vis_core.{h,cpp} = 954 + 4305 lines
(widget toolkit); gameplay.cpp 9364 (R5 exception, single class); ingame.cpp
8345 → 297 (gfx wrappers); main_window.cpp 6739 → 6439. Checker mismatch section:
only ScenarioLocation (documented). client_refs counts unchanged throughout:
3736 / 3412 / 3519.
