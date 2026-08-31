#include "visibility.h"

#include <cmath>
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

// 58CE74
void Visibility::sub_58CE74() {
    for (int32_t y = 0; y <= 20; y++) {
        for (int32_t x = 0; x <= 20; x++) {
            uint16_t p1;
            uint16_t p2;
            uint16_t p3;
            uint16_t p4;
            if (x < (y / 2)) {
                p1 = 0x00FF;
                p2 = 0x0001;
                p3 = 0x00FF;
                p4 = 0x0001;
            } else if (x > (y * 2)) {
                p1 = 0xFF00;
                p2 = 0x0100;
                p3 = 0x0100;
                p4 = 0xFF00;
            } else {
                p1 = 0xFFFF;
                p2 = 0x0101;
                p3 = 0x01FF;
                p4 = 0xFF01;
            }
            this->field_0x22000[y + 20][x + 20] = p1;
            this->field_0x22000[20 - y][20 - x] = p2;
            this->field_0x22000[y + 20][20 - x] = p3;
            this->field_0x22000[20 - y][x + 20] = p4;

            int32_t max_val = (y > x) ? y : x;
            int16_t value = 0;
            if (max_val != 0) {
                value = std::sqrt(y * y + x * x) * this->field_0x2a000 / max_val;
            }
            this->field_0x28000[y + 20][x + 20] = value;
            this->field_0x28000[20 - y][20 - x] = value;
            this->field_0x28000[y + 20][20 - x] = value;
            this->field_0x28000[20 - y][x + 20] = value;
        }
    }

    this->field_0x22000[21][20] = 0x00FF;
    this->field_0x22000[19][20] = 0x0001;
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
