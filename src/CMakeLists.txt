cmake_minimum_required(VERSION 3.20)
project(rea2 LANGUAGES C CXX RC)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MFC_FLAG 2)

set(A2_SOURCES
  action.cpp
  alm.cpp
  building.cpp
  buildings_list.cpp
  cmain.cpp
  effect.cpp
  eye.cpp
  file.cpp
  game_app.cpp
  gameplay.cpp
  gameobj.cpp
  gfx.cpp
  group.cpp
  ingame.cpp
  inn.cpp
  inventory.cpp
  item.cpp
  logic.cpp
  main_window.cpp
  map_stuff.cpp
  mouse.cpp
  net.cpp
  outpost.cpp
  packet.cpp
  perf.cpp
  player.cpp
  player_file.cpp
  players_list.cpp
  protections.cpp
  quest.cpp
  quest_glue.cpp
  quest_map.cpp
  resource.cpp
  sack.cpp
  scenario.cpp
  server.cpp
  shelf.cpp
  shop.cpp
  smacker_player.cpp
  shop_assortment.cpp
  spell.cpp
  spell_effect.cpp
  sound.cpp
  table.cpp
  take_damage.cpp
  token.cpp
  txtfile.cpp
  unit.cpp
  unit_list.cpp
  unit_to_hit.cpp
  util.cpp
  virtual_caster.cpp
  vis_char.cpp
  vis_core.cpp
  vis_fame.cpp
  vis_hat.cpp
  vis_inventory.cpp
  vis_menu.cpp
  vis_net.cpp
  vis_playing_area.cpp
  vis_shop.cpp
  vis_tavern.cpp
  vis_town.cpp
  vis_worldmap.cpp
  visibility.cpp
  world.cpp
  lib/smacker.c
  res/a2server.rc

  patch/locale.cpp
  patch/netlog.cpp
)

set(A2SERVER_PATCHES
  patch/allowed_picture.cpp
  patch/antiinvis.cpp
  patch/circle.cpp
  patch/config_new.cpp
  patch/crash_filter.cpp
  patch/ctl_handle.cpp
  patch/damage_filter.cpp
  patch/forbidden_items.cpp
  patch/gm_commands.cpp
  patch/gm_noclip.cpp
  patch/hat_net.cpp
  patch/hat_packet.cpp
  patch/kick_char.cpp
  patch/map_rotation.cpp
  patch/multiplayer_shop.cpp
  patch/partial_drop.cpp
  patch/player_info.cpp
  patch/player_settings.cpp
  patch/pvm2.cpp
  patch/quest_filters.cpp
  patch/reborn.cpp
  patch/scanrange.cpp
  patch/scanrange_calc.cpp
  patch/screenshots.cpp
  patch/server_state.cpp
  patch/solo.cpp
  patch/summon.cpp
  patch/tangar_log.cpp
  patch/tangar_utils.cpp
  patch/thresholds.cpp
)
add_executable(rea2s ${A2_SOURCES})
add_executable(rea2sp ${A2_SOURCES} ${A2SERVER_PATCHES})
add_executable(rea2st ${A2_SOURCES} ${A2SERVER_PATCHES})

set(A2CLIENT_PATCHES
  lib/happyhttp.cpp
  lib/lodepng.cpp
  patch/antiinvis.cpp
  patch/circle.cpp
  patch/client_config.cpp
  patch/config_new.cpp
  patch/crash_filter.cpp
  patch/ctl_handle.cpp
  patch/damage_filter.cpp
  patch/forbidden_items.cpp
  patch/gm_commands.cpp
  patch/gm_noclip.cpp
  patch/hat_client.cpp
  patch/hat_net.cpp
  patch/hat_packet.cpp
  patch/hotkeys.cpp
  patch/image.cpp
  patch/itempng.cpp
  patch/kick_char.cpp
  patch/locale.cpp
  patch/map_rotation.cpp
  patch/menu.cpp
  patch/multiplayer_shop.cpp
  patch/partial_drop.cpp
  patch/player_info.cpp
  patch/player_settings.cpp
  patch/pvm2.cpp
  patch/quest_filters.cpp
  patch/reborn.cpp
  patch/scanrange.cpp
  patch/scanrange_calc.cpp
  patch/screenshots.cpp
  patch/server_state.cpp
  patch/solo.cpp
  patch/stats_level.cpp
  patch/summon.cpp
  patch/tangar_log.cpp
  patch/tangar_utils.cpp
  patch/thresholds.cpp
)

add_executable(rea2c ${A2_SOURCES})
add_executable(rea2cp ${A2_SOURCES} ${A2CLIENT_PATCHES})
add_executable(rea2ct ${A2_SOURCES} ${A2CLIENT_PATCHES})

# Pin to what the build already defaulted to (26100 SDK MAXVER) — silences
# the "_WIN32_WINNT not defined" warning in every TU.
set(A2_COMPILE_DEFS
  _AFXDLL;FKG_FORCED_USAGE;_USE_32BIT_TIME_T;NDEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS;_WIN32_WINNT=0x0A00
)

foreach(TARGET rea2s rea2c rea2sp rea2st rea2cp rea2ct)
  set_property(TARGET ${TARGET} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
  )
  target_compile_definitions(${TARGET} PRIVATE ${A2_COMPILE_DEFS})
endforeach()

# Additional definitions for the server build:
# A2SERVER_PATCH: Enables various crash fixes.
# A2TANGAR_HAT: Enables the Tangar's hat balance changes (requires A2SERVER_PATCH).
target_compile_definitions(rea2sp PRIVATE A2SERVER_PATCH)
target_compile_definitions(rea2st PRIVATE A2SERVER_PATCH A2TANGAR_HAT)

# A2CLIENT_PATCH: Enables various crash fixes.
# A2TANGAR_HAT: Enables the Tangar's hat balance changes (requires A2CLIENT_PATCH).
target_compile_definitions(rea2c PRIVATE A2CLIENT)
target_compile_definitions(rea2cp PRIVATE A2CLIENT A2CLIENT_PATCH)
target_compile_definitions(rea2ct PRIVATE A2CLIENT A2CLIENT_PATCH A2TANGAR_HAT)

set(A2_LINK_LIBS
    kernel32.lib
    shell32.lib
    ddraw.lib
    user32.lib
    gdi32.lib
    comdlg32.lib
    winspool.lib
    msacm32.lib
    wininet.lib
    ws2_32.lib
    winmm.lib
    ole32.lib
    comctl32.lib
    dsound.lib
    ucrt.lib
    msvcrt.lib
    libvcruntime.lib
    legacy_stdio_definitions.lib
    vccorlib.lib
    advapi32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib
    dbghelp.lib
    dxguid.lib
    vcruntime.lib
    legacy_stdio_wide_specifiers.lib
    mfc140.lib
)

set(A2_LINK_OPTS
    /MANIFEST
    /NXCOMPAT
    /DYNAMICBASE
    /DEBUG
    /MACHINE:X86
    /SAFESEH
    /SUBSYSTEM:CONSOLE
    "/MANIFESTUAC:level='asInvoker' uiAccess='false'"
    /TLBID:1
    /NOLOGO
    /ERRORREPORT:PROMPT
)

foreach(TARGET rea2s rea2c rea2sp rea2st rea2cp rea2ct)
  target_link_libraries(${TARGET} PRIVATE ${A2_LINK_LIBS})
  target_link_options(${TARGET} PRIVATE ${A2_LINK_OPTS})

  target_compile_options(${TARGET} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:/MP>
  )

  target_link_options(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:/OPT:NOREF>
    $<$<CONFIG:Debug>:/OPT:NOICF>
  )
endforeach()
