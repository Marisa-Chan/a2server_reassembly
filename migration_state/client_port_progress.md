# Client patch port progress tracker
Updated: 2026-11-13 (all 16 postbuild/list.txt binary patches + ParseConfig/z_lang_override)

Legend: DONE / WIP / PENDING / NO-OP / DEFERRED(reason)

## Completed families
- CRASH batch 1: 46AD10 (DrawHealthBar), 420450 (GetPalette), 523295 (SetPower range) — 518559c9
- RES_loadFile override cascade — 8a88052e
- Client twins: INSTALLDIR cd, thread-priority x2 — 0892c9dd
- CHAT tilde x3 + logStrings + charset x2 (convWinToDos/convDosToIdx) — 05c53f94, 4ee9a961, b1083881
- Chat history subsystem (Up/Down/Enter) — e5caf6dd
- VIDEO_checkVideoMode smart default — f751620b
- character_stats ceiling 100 — 72a95477
- GUI_gmEnter GM bypass — 35a602e1
- LANG family (core + worldmap/famehall/town/mission/quest) — ae7e5d15, ecf6ab87
- Crash-guard quartet 4AAB7E/46B685/45FC3B/4657AD — e1650676, 37284f94 (incl. 458CF9 Blit16)
- Client Config subsystem (tangar_cfg: r_fps/r_time/r_stopwatch/r_grid/r_grid_color/
  g_scrollintv/z_language/stats_based_level_selection) — d14f576f
- SCROLL rate limiter (g_scrollintv) — 1183958f
- GRID tile overlay (r_grid, r_grid_color) — 8b39a278
- CRASH_fix_multiwindow 1-4: PID temp-file names + cleanup glob — 5c43e3ad
- ADMINISTRAT: staff nickname two-line draw in CUnit::DrawHealthBar — 420f6c42
  (client 46B17E; prefixes Administrator/GameMaster/TournamentMaster/BalanceMaker;
   title at y-0x19, char name at y-0xF, both in g_colors_human_pals[color])
- INFO_fps: config-gated fps/time/stopwatch HUD replacing retail g_ShowFps box — 7a8d9cdc
  (client 40AAF5..40ABDA in RenderWorldFrame; FONT_COLOR_GRAY=clrsh_TechBlack,
   WHITE=clrsh_ShockingBlack; RightShift keydown toggles/freeze/restarts stopwatch)

## Crash guards — all dispositioned
- NO-OP 414994 (ProcessPackets DIPLOMACY_STATE): hook = IsBadReadPtr chain pkt→[pkt]→flags&1;
  our vis_playing_area.cpp DIPLOMACY_STATE case already null-checks `mp` before `mp->flags & 1`
  and uses real CArray access — covered by construction.
- NO-OP 42353A (FUN_423530): CArray element-address thunk ([this+4] + idx*4) used by
  UpdateSelectionState/Spell::Init/SetPower; binary guard probes a raw pointer our C++ never
  walks — MFC CArray owns its storage; no port possible or needed.
- NO-OP 4BEE2D (VisShop ctor zero-init 0x164..0x17C): covered-by-construction.

## GUI softcore family — DONE
- GUI_softcoreEnter (44DD0F) stats-based level selection — fef03834: patch/stats_level.{h,cpp}
  (ParseServerNumber + brackets by FileSectionStats: <15 mind / <20 reaction / 20-30 / 30-40 /
  40-50 partial / 50+50+50), creating_new_character set in VisCharGenStats::LoadBitmaps (42B599),
  cleared in ParsePlayerFile (4F75F5); stats read via GetParsedPlayerStats() (s_stats_buf,
  client 0x6995a8 twin); tangar_cfg::stats_based_level_selection now defaults TRUE (a2mgr parity)
- Dropdown rework — 4de26160: sandboxSwitch single "hardcore" entry radio(3,l,0x7C,r,0xDC);
  ResetSwitch z_softcore override + remap (2->1,4->2); softcoreSwitch write remap (1->2,2->4);
  SaveSwitch deathmatch registry remap when z_softcore forces mode; removeServerList = off-screen
  disabled id-4 radio (flags=0, entries+ReadData kept); setServColumnSize list top 0x5c->0x44 +
  non-dm2 bottom H-0x80->H-0x60. NOTE: our VMethod26 previously carried the PATCHED values
  ungated (0x44/H-0x60) — now retail (0x5c, H-0x80 client / H-0x60 server) sits under #else.
  z_softcore has NO config parse in a2mgr (always 0 → override paths dormant); knob added anyway.

## Session batch (2026-11-12, token-max cont. 2) — inventory sweep COMPLETE
- hotkeys x10 FULL SUBSYSTEM (29d89773..31c5ec6e): patch/hotkeys.{h,cpp} (aliases config
  "alias:<key> <action>", labels, hotkey.<id1>.a2key persistence w/ retail UserShortcut
  serialization, intercept helpers), sites: VisPlayingArea::OnKeyDown (head intercept +
  TangarHandleHotkey = F-key case driven by custom map), MainWindow::OnKeyDown (TopLevelKeyDispatch
  -> vis_root MsgProc / HELP / DIPLOMACY), VisInvType1::MsgProc 0x417 (item find/set),
  VisSpellBook::MsgProc 0x417 (spell use/set x3), label draws (items/spells), VisCharInfo
  (panel_toggle + is_spell_book_open tracking). Hotkey = our UserShortcut; key encoding:
  Action = VK of the retail case body (patch used raw addresses); MsgProc 0x417 wparam =
  key - VK_F4 (can be negative for non-F keys - hooks recover VK = wparam + VK_F4).
- pack_scrolling (020c51b3): VisInvType1::OnKeyDown new override (PageUp/Down bag paging,
  steps = (scrW-240)/80-1), VisInvExtType1::OnKeyDown Ctrl-gated.
- info_panel_left_sprite (eaa85151): lower info strip pinned to window bottom.
- info_experience_spaces (9652d0d9): thousands separators in stats panel (gameobj.cpp).
- fix_StoneCurseDurationCalculation (dbe249ec): spell.cpp SetPower stone curse
  power*info.duration*32/100 (chain [this+4]->spell_info, +0xC values.m_pData, +0x38
  = SpellInfoData[0].duration index 14).
- GUI_textField_MeasurePosition (71fba0a6): VisTextBox::GetCursorPosByX rewrite
  (accuracy + password asterisks + non-zero X).
- HATENT_url_text/url_connect (cdef809c): tangar_cfg::HatAddress() = -hat cmdline arg
  (default hat.rom2.ru); overrides hatip registry read + PrepareForConnect target.

## NO-OP'd (verified not needed)
- FIL x13 (ord2-14) + RES_loadDirectories/RES_noLoad/RES_loadException (resource family),
  fixEscape, getMessageLast('/'), wmChar(baseline)
- [DISABLED] in the patch itself: TILES_loadTiles/displayTile1, ord96, CHAT_dbg, DBG_x5
  (CSprite256 debug)
- _fix_launcher (patcher-side unpacker wait tool)

## Session batch 8 (2026-11-13) — all 16 `postbuild/list.txt` binary patches & `z_lang_override` COMPLETE
- `ParseConfig` + `z_lang_override` (`bb19f89e`): call `tangar_cfg::ParseConfig()` on startup in
  `GameApp::InitInstance` before `DecideLanguage()`, accept `"rus"`/`"russian"` and `"eng"`/`"english"`
  (else `z_lang_override = false`, default `z_russian = true`), and sync `tangar_cfg::z_russian` in
  `tangar_locale::DecideLanguage()`.
- `boneidle.xck` (`f10cbdf0`): use `dying_vfx->idle_phases` (`+0x28`) instead of `bone_phases` (`+0x24`)
  for corpse direction stride in `CUnit::Draw` (`468BD9`, `468C34`) and `CUnit::DrawShadow` (`469C2F`, `469C87`).
- `dem_shop.xck` (`ed5c5f8b`): silence Druid shop `snd_bird[0..2]` and `snd_tool[0..3]` load/play/free in
  `VisShopDruid::VMethod31`/`VMethod30`/`VMethod32` while keeping RNG/timer updates intact.
- `extend_servers.txt` (`b0457718`) + `long_servers.txt` (`7e28c0f4`): `VisHatBrowserDlg` enlarged to
  `704x544` (`48DEAD`, `48DEB2`), header labels shifted up to `0x10`/`0x20` (`44E7E7`, `44E87F`), and
  button row recentered via `OffsetRect(4, 0)` (`44EF07`); list/caption/status positions already ported.
- `long_dip.txt` (`c51c75b6`): `VisDiplomacy` height `480 -> 592` (`48B423`), single-line `VisMessageBox`
  payload bottom `Height - 80` (`446875`), `VisDiplomacy` bottom margin `-= 32` (`446A03..446A15`), and
  `VisListBoxDiplomacy` `entry_height_full = _entry_height + 1` (`4E13FF`).
- `p10ach.xck` (`423422a9`): disable `sec->hat_id == 0xbabad0d0` wildcard character roster fallback in
  `CGameSession::RefreshCharacterRosterFiles` (`497083`).
- `p10iti.xck` (`a6059359`): enlarge `CMousePointer` `hint_image` and `saved_screen_rect_hint` `CBmp64`
  buffers from `640x117` to `640x453` (`42859B`, `4285DD -> 6E2213`).
- `p10itm.xck` (`4ade69f0`): add Alt=`min(count, 1000)` and Ctrl=`min(count, 10)` item drag modifiers in
  `VisInvType1::OnMouseMove` (`4AB2A8 -> 6E21D0`) and `VisInvExtBase::OnMouseMove` (`4BA637 -> 6E21D0`).
- `p10lck.xck` (`aa87aeae`): lock `HatSettings::Load` to `ishat = 1` (`43F895`, `43F8B6`), set
  `ip_box->flags = CVisualObject::FLAG_NOTFOCUS` in `VisHatServerListDlg::VMethod30` (`44CAAD -> 6E215B`),
  and fix `HATENT_url_text` (`43F738`) target field from `hatip` (`+0`) to `hatprogip` (`+4`).
- `p10mny.xck` (`0c399754`): skip money slot (`item_id == 0xFFFF`) before `MatchesItem` in
  `VisInvType1::MsgProc` (`4AB621 -> 6E20EE`).
- `p10st2.xck` (`e48209ae`): reject `spell_id == 0 || spell_id > 0x1D` (`spell::max_spell_id`) in
  `Spell::Init` (`522F14 -> 6E20F9`) and `Effect::CalcScrollPrice` (`52916F..529195`).
- `p10stf.xck` (this commit): guard `spell.spell_info != nullptr` before `SetPower` and damage draw in
  `CUnit::DrawStatsPanel` (`470452 -> 6E20C0`).
- Verified already covered / NO-OP: `p10ncd.xck` (`g_IsCdPresent = 1` in `game_app.cpp`), `p10reg.xck`
  (`SaveSettings` already uses `KEY_WRITE` + VirtualStore; `LoadSettings`/`InitInstance` stay `KEY_READ`
  under UAC `asInvoker`), `showmap.xck` (session batch 5), and `rdata_writable.txt` (PE `.rdata` flag for
  runtime DLL hooks).

## Session batch 7 (2026-11-13) — v20 auth protocol, screenshot net & HTTP auto-updater COMPLETE
- v20 HAT authorization + crypt + extended TCP framing (d8947292):
  `patch/hat_client.{h,cpp}` (`EnsureAnticheatHashes` SHA1/CRC32, `BuildAuthorizationPacket`
  id=0 layout +0x0A..+0x50 + `crcmain` gamemode -> `hat_settings.deathmatch = 0` &
  `tangar_cfg::z_softcore`, `RecvClientFramed` 0x80100000 demux + synthetic 0x64 `SERVER_TICK`),
  `XorData` -> `PACKET_XorByKey(..., 20)`, `PacketJoin` id=0 read/write/size,
  `DecodePacket` replaces `HAT_CREDENTIALS` (0xC9) with `0` under patch `#if`,
  51200B `RecvThreadTcp` buffer + server `RecvServerFramed` & `FlushEnqueuedPackets`
  (`PlayerInfo::EnqueuedPackets`), and fixed `HATENT_url_connect` placement from
  `RunModalLoopServer` to `RunModalLoopClient`.
- HAT-triggered screenshot capture + PNG upload + `#screenshot` (08196993):
  vendored `patch/happyhttp/` (with 4s non-blocking connect timeout), client `0x80100000`
  opcode `0x01` + `imp_MakeScreenshotNet` (`UnlockSurface1` @ 457FF4) + background thread
  notifying `hat:8000` (protover 24 magic `0x5C0EE250`) and uploading PNG via HTTP multipart
  POST to `api.allods2.eu:8080/posts.php`; server `patch/screenshots.{h,cpp}`, HAT packet
  `0x64` completion reply, `PI_Delete` cleanup, and `#screenshot <nick>` (`GMF_CMD_SCREENSHOT`).
- HTTP auto-updater + `-delay` startup flag (948326b3):
  `tangar_client::RunAutoUpdater` in `RunModalLoopClient` (`/allods/version_id` +
  `/allods/version` check on `api.allods2.eu:8080`, graceful pass-through when unreachable/empty,
  per-file HTTP download on port 80 into `data/` with main-thread progress UI, self-restart
  with `-delay 4000`), and `-delay <ms>` handling in `GameApp::InitInstance`.

## Session batch 6 (2026-11-13) — scanrange pair COMPLETE (019b53d8)
- patch/scanrange_calc.{h,cpp} — CScanrangeCalc verbatim from
  my-srvmgr/lib (41x41 offset/cost/vision tables; ulScanShift=7 = the retail
  shadow_shift; cost = sqrt(i^2+j^2)*128/max(i,j)).
- Client CalcVisMapCLIENT (client 403D47) = BuildShadowField #ifdef branch:
  CalculateVision(obj->tileX/tileY/scan_range) + memcpy into shadow_light;
  helpers read scenario->GetMapHeights()/GetWidth() and map_width/height-7
  bounds (note: client check uses '<', server uses '<=').
- Server SR_* (patch/scanrange.{h,cpp}, A2SERVER_TANGAR_HAT via
  A2SERVER_SOURCES): SR_Step from ServerTic (after NetRegularProc, before
  Sleep(1), matching OnServerTic order); SR_UpdateUnit stamps
  GetTickCount() into PlayerInfo.Vision[256][256] (new field) for the
  unit's owner + vision-share partners (diplomacy[a][b] & DIPLO_VISION_SHARE
  = 0x10, row = owner); heights from MapStuff_Instance->height_map
  (256-stride, @0x944F4), bounds from map_width/map_height (@0x50000);
  non-main units first, then mains. SR_CheckVision ported for parity
  (unused in the manager too). #scan (GMF_CMD_INFO) dumps the vision grid
  to the log via SR_DumpToFile; #info/#locate stay deferred (pvm2).

## Session batch 5 (2026-11-13) — map download + showmap
- MAP_CFileOpen1/2 + MAP_loadFromServer (db9afb67): FILE_CHUNK handler in
  VisPlayingArea::ProcessPackets stores downloaded maps under maps\ —
  TangarMapsPath() prefixes once up front (covers both chunk-write opens),
  final block becomes DeleteFile+MoveFile into maps\. Server side unchanged:
  MAP_INFO announces current_map_name verbatim (config [maps] entries listed
  as "maps\..." close the loop), FILE_CHUNK preamble is always the stripped
  tail. MAP_CalcCRC hook was never active (and check_map_file is a stub) —
  the checksum path needs no port.
- showmap.xck — MISSED by the inventory sweep (active xck, not in any
  disposition list), found via the 40F264 flip grouped next to the MAP
  hooks. 4 byte-flips = MAP_REVEAL becomes a flag-only observer marker:
  case 1 no longer stamps landscape |= 0xc000 (40F264), revealed map no
  longer forces stats view_level 7 in CUnit::DrawStatsPanel (46FF75) and
  CUnit::GetStatsHint (470F2E), occupancy rebuild runs even while revealed
  (40BE1B). Ported in A2CLIENT_PATCH blocks; retail under #else/#ifndef.
  Likely anti-spectator-intel for HAT (no permanent fog reveal / enemy
  stats for dead observers).
- Housekeeping: src/vis_playing_area.cpp was the repo's last CRLF blob —
  normalized to LF in a standalone commit (1f8ee143) before the map port
  (db9afb67) so history stays bisectable.

## Session batch 4 (2026-11-12) — SDL/PNG asset cluster COMPLETE
- patch/image.{h,cpp} — Tangar Image port (SDL dropped): PNG via vendored
  lodepng + built-in uncompressed 24/32-bit BMP decoder; RGB565 blend onto
  g_selDrawBitmap, manager clipping verbatim. Fixed the manager's
  transparent-pixel dst-pointer stall (unreachable with shipped assets).
- MENU x6 (53c10848): custom main menu = data\locale\{ru,en}\graphics\
  mainmenu\menunew.png (640x480) + 3 inn-button buttons at (251,258/304/351),
  140x46 hit rects. Labels = string-table lines 238-240: Play/Authors/Quit
  (CP866 RU literals; patch.txt is 0-based). Dispatch via retail button codes
  5 (HAT server list) / 3 (credits) / 7 (WM_CLOSE) written to active_button
  on left-down; OnLButtonUp's retail switch fires on left-up, then hover3
  resets. LoadGraphics/VMethod7 bodies fully replaced (even dialogsMask check
  and ClientRectToScreen are skipped, per hook targets); OnLButtonUp only
  swaps UpdateButtonState for the hover. MENU_click = dead-code no-op in the
  release patch (its DoMenuClick block is unreachable). Sound = retail
  CSound::Play((CSound&)snd_btn_click) from the hit-test. Fonts: g_font4 +
  palette_husk (gold) / palette_paris_daisy (goldbright) via GetPalette(0).
- ITEMPNG x3: PatchPngSprite : CSprite256 (patch/itempng.{h,cpp}) overrides
  Draw (slot 6) + DrawIndexMask (slot 16) with the manager's IImage_Display/
  CopyTo math (160x240-only, index-based hitmask writes, max(...,1) clamps);
  no-op operator delete = the cachePng ??_G guard covered-by-construction;
  permanent filename cache incl. negative entries. Sites: RenderHeroPortrait
  equipment primary/secondary + face sprite via TangarLoadItemSprite(.256
  path -> data\ twin .png). gfx.h gains a protected CSprite256() default ctor
  under A2CLIENT_PATCH.
- add_info console UI: NO-OP — ConsoleImpl() is commented out inside the
  release FpsImpl and no console PNG assets ship; the only live part (Right-
  Shift stopwatch via CHAT_fixEscape) is already ported with INFO_fps, and
  the Shift+~ toggle flips a variable nothing reads.

## Session batch 3 (2026-11-12)
- SCREENSHOT_take (2110b2e5): PNG screenshots via vendored lodepng (src/patch/lodepng/,
  zlib-licensed, same file the manager uses), z_screendir knob
- CHAT_prependType x2 (ffabfe44): [Say/Private/Shout] prefixes under g_altchat
  (patch.res lines 223-225 approximated with locale-aware literals)
- Server #-commands reconciled: #mapinfo (9ccbbc07), #killai (5adecc77),
  #mapwide + #inn (b00607e7), #vote_map (c75f14fb), #reborn + #hell
  (3dd1723b, with the thresholds.cfg parser + reborn readiness module) — memory s37/s38

## DEFERRED (subsystem pairings)
- (v20 auth + NET_cryptPacket + recv_ + imp_MakeScreenshotNet + #screenshot + HTTP auto-updater
  now ported in session batch 7: d8947292, 08196993, 948326b3)
- (CHAT_prependType x2 now ported with hardcoded locale strings)
- (MENU x6 + ITEMPNG x3 + add_info console UI done/no-op'd, session batch 4)
- (MAP_CFileOpen1/2 + MAP_loadFromServer + showmap.xck done, session batch 5;
  the 0xBF/0x3B transfer itself was already in — server side needs only
  "maps\..." [maps] config entries)
- (scanrange pair + #scan done, session batch 6; remaining #-commands:
  #info (pvm2 CheckStrong), #murder (DropEverything))
