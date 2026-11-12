# Client patch port progress tracker
Updated: 2026-11-12 (token-max session, cont.)

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

## Session batch 3 (2026-11-12)
- SCREENSHOT_take (2110b2e5): PNG screenshots via vendored lodepng (src/patch/lodepng/,
  zlib-licensed, same file the manager uses), z_screendir knob
- CHAT_prependType x2 (ffabfe44): [Say/Private/Shout] prefixes under g_altchat
  (patch.res lines 223-225 approximated with locale-aware literals)
- Server #-commands reconciled: #mapinfo (9ccbbc07), #killai (5adecc77),
  #mapwide + #inn (b00607e7), #vote_map (c75f14fb) — see memory session-37

## DEFERRED (subsystem pairings)
- CalcVisMapCLIENT (scanrange.cpp) — pairs with the unported SERVER SR_Step/OnServerTic
  scanrange subsystem; port both together (terrain-aware vision)
- MAP_CFileOpen1/2 + MAP_loadFromServer (map_location.cpp "not used") — maps\ download
  redirect, pairs with the custom-map sync work (0xBF/0x3B, OPEN in memory)
- NETPROTO x3 + NET_cryptPacket + recv_ — v20 auth/protocol (spec in memory, user-shelved)
- MENU x6 + ITEMPNG x3 + add_info console UI — SDL/PNG asset dependency
- imp_MakeScreenshotNet — HAT-net-triggered screenshot (0x64 packet, with v20 auth)
- (CHAT_prependType x2 now ported with hardcoded locale strings)

## DEFERRED
- PNG x3 + ITEMPNG (SDL), add_info console UI (PNG assets)
- NETPROTO x4 + HAT v20 auth (user decision spec in memory), updater (with auth), screenshots
