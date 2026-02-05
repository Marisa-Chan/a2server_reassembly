#pragma once

#include <cstdint>

#include "assert_offset.h"

struct Perf {
    int64_t begin; // These are likely `LARGE_INTEGER` in the original code, but I don't want to include Windows headers here.
    int64_t end;
    int64_t freq64;
    uint32_t period;
    uint32_t cnt;
    uint32_t freq;
    uint8_t gap_0x24[4];
    int64_t period64;
};
ASSERT_OFFSET(Perf, period, 0x18);
ASSERT_SIZE(Perf, 0x30);
