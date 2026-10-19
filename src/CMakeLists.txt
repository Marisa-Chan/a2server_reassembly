cmake_minimum_required(VERSION 3.20)
project(rea2 LANGUAGES CXX RC)

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
  game.cpp
  game_app.cpp
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
  perf.cpp
  packet.cpp
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
  shop_assortment.cpp
  spell.cpp
  spell_effect.cpp
  sound.cpp
  table.cpp
  token.cpp
  txtfile.cpp
  unit.cpp
  unit_list.cpp
  unit_to_hit.cpp
  util.cpp
  virtual_caster.cpp
  visibility.cpp
  visual.cpp
  world.cpp
  res/a2server.rc
)

set(A2SERVER_SOURCES
  patch/kick_char.cpp
  patch/allowed_picture.cpp
)

add_executable(rea2s ${A2_SOURCES} ${A2SERVER_SOURCES})
add_executable(rea2c ${A2_SOURCES})

# Pin to what the build already defaulted to (26100 SDK MAXVER) — silences
# the "_WIN32_WINNT not defined" warning in every TU.
set(A2_COMPILE_DEFS
  _AFXDLL;FKG_FORCED_USAGE;_USE_32BIT_TIME_T;NDEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS;_WIN32_WINNT=0x0A00
)

foreach(TARGET rea2s rea2c)
  set_property(TARGET ${TARGET} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
  )
  target_compile_definitions(${TARGET} PRIVATE ${A2_COMPILE_DEFS})
endforeach()

target_compile_definitions(rea2c PRIVATE A2CLIENT)

# Additional definitions for the server build:
# A2SERVER_PATCH: Enables various crash fixes.
# A2SERVER_TANGAR_HAT: Enables the Tangar's hat balance changes (requires A2SERVER_PATCH).

target_compile_definitions(rea2s PRIVATE A2SERVER_PATCH A2SERVER_TANGAR_HAT)

# A2CLIENT_PATCH: Enables various crash fixes.
target_compile_definitions(rea2c PRIVATE A2CLIENT_PATCH)

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

foreach(TARGET rea2s rea2c)
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
