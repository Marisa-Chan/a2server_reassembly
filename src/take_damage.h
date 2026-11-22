#ifndef A2_TAKE_DAMAGE_H
#define A2_TAKE_DAMAGE_H

#pragma once
#include <cstdint>
#include <afx.h>
#include <afxtempl.h>
#include "assert_offset.h"

class CUnit;

struct TakeDamage {
    CString txt;
    int32_t dmg = 0;
    uint16_t* color = nullptr;
    int32_t dx = 0;
    int32_t dy = 0;
    uint32_t timestamp = 0;
    int32_t drift_left = 1;
    CUnit* cunit = nullptr;
    int32_t burst_id = 0;

    TakeDamage(); // 45db60 (client: 461780)
    TakeDamage(int32_t dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct); // 45db8a (client: 4617aa)
    TakeDamage(const TakeDamage& ref); // C++ copy ctor
    ~TakeDamage(); // 45dd08 (client: 461928)

    void Update(); // 45dd1b (client: 46193b)
    int Draw(); // 45dd6d (client: 46198d)

    int32_t GetScreenX(); // 41f620 (client: 4203d0)
    int32_t GetScreenY(); // 41f640 (client: 4203f0)

    static void __cdecl AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg); // 45dea1 (client: 461ac1)
    static void __cdecl DrawDamages(CArray<TakeDamage>* arr); // 45ddf6 (client: 461a16)
    static void __cdecl UpdateDamages(CArray<TakeDamage>* arr); // 45de65 (client: 461a85)
#ifdef A2CLIENT_PATCH
    // Crash fix (deviation from retail): labels hold a raw CUnit* for up to 1s;
    // retail deletes units (TOKEN_REMOVE, deep death, player leave, per-frame
    // object expiry) without purging them - use-after-free in Draw().
    static void __cdecl RemoveDamagesFor(CArray<TakeDamage>* arr, CUnit* ct);
#endif
};
ASSERT_SIZE(TakeDamage, 0x24);

#endif
