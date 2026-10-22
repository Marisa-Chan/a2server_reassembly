#pragma once

#include <cstddef>

// Intentionally incomplete.
template <std::size_t Expected, std::size_t Actual>
struct GotWant;

// Only defined for equality.
template <std::size_t X>
struct GotWant<X, X> {
    enum {
        ok = 1,
    };
};

// These check a field offset / class size at compile time. `expected` may be
// any constant expression, including references to sizeof() of a base class:
//
//   ASSERT_SIZE(MyClass, 0x40 + sizeof(CFrameWnd));
//
// This matters because the migrated C++ links MFC 140 while the original
// binaries were built against VC6 MFC: every CWnd-family base (CWnd, CFrameWnd,
// CWinApp, CDialog, CListBox, CEdit, CStatusBar, ...) is LARGER in MFC 140, so
// absolute offsets from the binary are wrong for those classes and asserts
// must be written relative to the current base size. CObject-rooted classes
// are unaffected (CObject is a bare vtable pointer in both MFC versions), so
// plain absolute values from the binary are correct there.
//
// For fields that sit AFTER an embedded CWnd-family member the simple
// sizeof(base) + delta formula no longer holds (the embedded member's growth
// and new alignment padding shifts everything behind it); chain such asserts
// off the preceding field instead:
//
//   ASSERT_OFFSET(MainWindow, sessionMode, OFF(MainWindow, serverBootstrapEnabled) + 4);

#define ASSERT_OFFSET(type, field, expected) \
    static_assert(offsetof(type, field) == (expected), #type "::" #field " offset mismatch"); \
    static_assert(GotWant<offsetof(type, field), (expected)>::ok, #type "::" #field " offset")

#define ASSERT_SIZE(type, expected) \
    static_assert(sizeof(type) == (expected), #type " size mismatch"); \
    static_assert(GotWant<sizeof(type), (expected)>::ok, #type " size")
