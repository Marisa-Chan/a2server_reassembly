#include "visibility.h"

#include <cstring>
#include <memory>

#include "file.h"
#include "resource.h"

// 58E1C1
void Visibility::sub_58E1C1() {
    memset(this->field_0x2a008, 0, sizeof(this->field_0x2a008));
}

// 58DE69
void Visibility::sub_58DE69(Unit* unit, uint16_t yx) {
    this->sub_58D768(unit, yx);
}

// 58CD84
void Visibility::sub_58CD84() {
    for (int32_t y = 0; y <= 20; y++) {
        for (int32_t x = 0; x <= 20; x++) {
            int16_t dist_sq = static_cast<int16_t>(y * y + x * x);
            this->field_0x20000[y + 20][x + 20] = dist_sq;
            this->field_0x20000[y + 20][20 - x] = dist_sq;
            this->field_0x20000[20 - y][x + 20] = dist_sq;
            this->field_0x20000[20 - y][20 - x] = dist_sq;
        }
    }
}

// 58E1E4
Visibility::Visibility() {
    File2 f;
    f.Open("World\\Data\\map.reg", 0, nullptr);

    RegFile reg;
    reg.ReadFromFile(&f, -1);

    int32_t scan_shift = reg.GetInt("Scanning", "ScanShift", 7);

    this->scan_shift = scan_shift;
    this->field_0x2a000 = 1 << scan_shift;

    this->sub_58CD84();
    this->sub_58CE74();

    memset(this->field_0x0, 0, sizeof(this->field_0x0));
}
