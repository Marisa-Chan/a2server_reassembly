#include "visibility.h"

#include <memory>

#include "file.h"
#include "resource.h"

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
