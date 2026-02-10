cmake_minimum_required(VERSION 3.20)
project(a2serv_vsc LANGUAGES CXX RC ASM_MASM)

set(CMAKE_CXX_STANDARD 17)

add_executable(a2serv_vsc
  alm.cpp
  asm_mfc.cpp
  building.cpp
  cmain.cpp
  effect.cpp
  eye.cpp
  file.cpp
  file_mfc.cpp
  game.cpp
  group.cpp
  inn.cpp
  inventory.cpp
  item.cpp
  logic.cpp
  map_stuff.cpp
  net.cpp
  outpost.cpp
  perf.cpp
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
  strcore.cpp
  strex.cpp
  table.cpp
  testre.cpp
  token.cpp
  unit.cpp
  unit_list.cpp
  unit_to_hit.cpp
  virtual_caster.cpp
  world.cpp
  Main.asm
  a2_ucrt.asm
  vtbl_ret.asm
  res/a2server.rc
)

target_include_directories(a2serv_vsc PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/mfc/include"
)

set_property(TARGET a2serv_vsc PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

target_compile_definitions(a2serv_vsc PRIVATE
  NDEBUG;_CONSOLE;_CRT_SECURE_NO_WARNINGS
)

target_link_libraries(a2serv_vsc PRIVATE
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

    vcruntime.lib
    legacy_stdio_wide_specifiers.lib
)

target_link_options(a2serv_vsc PRIVATE
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

# Enable MASM51 compatibility and SAFESEH.
target_compile_options(a2serv_vsc PRIVATE
  $<$<COMPILE_LANGUAGE:ASM_MASM>:/Zm;/safeseh>
)
