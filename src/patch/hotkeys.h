#pragma once

// Tangar custom hotkeys, ported from my-a2mgr hotkey.{h,cpp} + the alias part of
// config.cpp. Two features layered over the retail UserShortcut F4-F12 system:
//  - any key can be bound to an item/spell (persisted per character in
//    hotkey.<id1>.a2key, same serialization as retail shortcuts);
//  - any key can be aliased to a retail action key (allods2.cfg "alias:<key> <action>").

#include <cstdint>
#include <map>
#include <string>

class UserShortcut;

namespace tangar_hotkeys {

// The patch encoded actions as client code addresses (jump targets inside
// VisPlayingArea::OnKeyDown / MainWindow::OnKeyDown). Here an action is simply
// the retail virtual key whose case body the patch jumped into; the hooked
// dispatch re-enters the retail switch with that key.
enum class Action : uint32_t {
    DEFAULT = 0,

    // Top-level actions, handled in MainWindow::OnKeyDown.
    HELP = 0x70,      // VK_F1 (patch: 0x0048a261)
    DIPLOMACY = 0x72, // VK_F3 (patch: 0x0048a277)

    // Unit actions, cases of the VisPlayingArea::OnKeyDown switch.
    UNIT_A = 'A', // attack       (patch: 0x0040cce3)
    UNIT_C = 'C', // center       (patch: 0x0040cd47)
    UNIT_D = 'D', // disband      (patch: 0x0040cd67)
    UNIT_G = 'G', // guard        (patch: 0x0040ccf7)
    UNIT_L = 'L', // (unit)       (patch: 0x0040cd8d)
    UNIT_M = 'M', // move         (patch: 0x0040cccf)
    UNIT_P = 'P', // patrol       (patch: 0x0040cd7b)
    UNIT_R = 'R', //              (patch: 0x0040cd1f)
    UNIT_S = 'S', // stop         (patch: 0x0040cd33)
    UNIT_T = 'T', // (unit)       (patch: 0x0040cd0b)

    // Other actions, also cases of the VisPlayingArea::OnKeyDown switch.
    ACTION_B = 'B', // (patch: 0x0040ce12)
    ACTION_C = 'C', // (patch: 0x0040d2a5)
    ACTION_E = 'E', // (patch: 0x0040d27e)
    ACTION_F = 'F', // (patch: 0x0040d016)
    ACTION_H = 'H', // (patch: 0x0040d05e)
    ACTION_I = 'I', // (patch: 0x0040cdcd)
    ACTION_K = 'K', // (patch: 0x0040d222)
    ACTION_L = 'L', // (patch: 0x0040d12e)
    ACTION_N = 'N', // (patch: 0x0040d178)
    ACTION_O = 'O', // (patch: 0x0040d1d2)
    ACTION_T = 'T', // (patch: 0x0040d288)
    ACTION_U = 'U', // (patch: 0x0040d0a8)
    ACTION_W = 'W', // (patch: 0x0040cfaf)
};

// Config: "alias:<name> <action>" line from allods2.cfg (both already lowercased).
void ParseAlias(const std::string& name, const std::string& action);

// Custom item/spell hotkeys (hotkey.<id1>.a2key).
void LoadCustomHotkeys();
void SaveCustomHotkeys();
UserShortcut* LookupHotkey(int key); // custom hotkeys only, nullptr if unset

// Key label for the on-screen hotkey markers ("F5", "Q", "NUM 7", ...).
const char* Label(int key);

// True while the game world is up and a hero is loaded (patch HotkeysAvailable,
// tracked via the hooked OnKeyDown entry points).
bool HotkeysAvailable();
void RememberKeyHandler(void* obj);
void RememberWindow(void* window);

// Spell book visibility, tracked by the is_spell_book_open hook.
void SetSpellBookOpen(bool open);
bool SpellBookOpen();

// Action classification (see the Action enum for the patch's address encoding).
bool IsTopLevelAction(Action action);
bool MaybeUnitAction(Action action);
bool MaybeGlobalAction(Action action);

// Key-intercept entry points used from the hooked OnKeyDown flows.
bool ShouldHandleHotkey(int key);   // custom item/spell hotkey handles this key
int HandleAliasKey(int key);        // aliased retail action key to run instead, 0 = none
// 0 = normal flow, -1 = forward to the standard in-game key handler, else the
// top-level action key (HELP/DIPLOMACY) to dispatch directly.
int32_t TopLevelKeyDispatch(int key);

// Mouse/state predicates (patch MouseOverMagic/SpellHighlighted/...).
bool SpellHighlighted();
bool MouseOverMagic();
bool InventoryOpen();
bool MouseOverItem();
void RememberPanel(void* panel);

} // namespace tangar_hotkeys
