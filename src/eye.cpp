#include "eye.h"


// 5A4F30
UnitEye2::~UnitEye2()
{
    delete positions_list;
    positions_list = nullptr;
}
