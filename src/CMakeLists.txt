cmake_minimum_required(VERSION 3.20)
project(a2serv_vsc LANGUAGES CXX RC ASM_MASM)

set(CMAKE_CXX_STANDARD 17)

set(A2_SOURCES
  alm.cpp
  asm_mfc.cpp
  building.cpp
  cmain.cpp
  effect.cpp
  eye.cpp
  file.cpp
  file_mfc.cpp
  game.cpp
  game_app.cpp
  gfx.cpp
  group.cpp
  ingame.cpp
  inn.cpp
  inventory.cpp
  item.cpp
  logic.cpp
  main_window.cpp
  map_stuff.cpp
  mfc_coll.cpp
  mouse.cpp
  net.cpp
  outpost.cpp
  perf.cpp
  packet.cpp
  player.cpp
  player_file.cpp
  players_list.cpp
  plex.cpp
  protections.cpp
  quest.cpp
  quest_glue.cpp
  quest_map.cpp
  resource.cpp
  sack.cpp
  server.cpp
  shelf.cpp
  shop.cpp
  shop_assortment.cpp
  spell.cpp
  spell_effect.cpp
  sound.cpp
  strcore.cpp
  strex.cpp
  table.cpp
  token.cpp
  txtfile.cpp
  unit.cpp
  unit_list.cpp
  unit_to_hit.cpp
  util.cpp
  virtual_caster.cpp
  visual.cpp
  world.cpp
  Main.asm
  a2_ucrt.asm
  res/a2server.rc
)

add_executable(a2serv_vsc ${A2_SOURCES})
add_executable(a2client_vsc ${A2_SOURCES})

set(A2_COMPILE_DEFS
  FKG_FORCED_USAGE;_USE_32BIT_TIME_T;NDEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS
)

foreach(TARGET a2serv_vsc a2client_vsc)
  target_include_directories(${TARGET} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/mfc/include"
  )
  set_property(TARGET ${TARGET} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
  )
  target_compile_definitions(${TARGET} PRIVATE ${A2_COMPILE_DEFS})
endforeach()

target_compile_definitions(a2client_vsc PRIVATE A2CLIENT)

# Additional definitions for the server build:
# A2SERVER_PATCH: Enables various crash fixes.
# A2SERVER_TANGAR_HAT: Enables the Tangar's hat balance changes (requires A2SERVER_PATCH).

target_compile_definitions(a2serv_vsc PRIVATE A2SERVER_PATCH A2SERVER_TANGAR_HAT)

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
)

set(A2_LINK_OPTS
    /MANIFEST
    /NXCOMPAT
    /DYNAMICBASE
    /DEBUG
    /MACHINE:X86
    /OPT:REF
    /OPT:ICF
    /SAFESEH
    /SUBSYSTEM:CONSOLE
    "/MANIFESTUAC:level='asInvoker' uiAccess='false'"
    /TLBID:1
    /NOLOGO
    /ERRORREPORT:PROMPT
)

foreach(TARGET a2serv_vsc a2client_vsc)
  target_link_libraries(${TARGET} PRIVATE ${A2_LINK_LIBS})
  target_link_options(${TARGET} PRIVATE ${A2_LINK_OPTS})

  # Enable MASM51 compatibility and SAFESEH.
  target_compile_options(${TARGET} PRIVATE
    $<$<COMPILE_LANGUAGE:ASM_MASM>:/Zm;/safeseh>
  )
endforeach()
