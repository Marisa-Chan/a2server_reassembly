#include "virtual_caster.h"

IMPLEMENT_SERIAL(VirtualCaster, Token, 1); // Runtime class definition at 6363d0.

// 528900 (client: 517649)
VirtualCaster::VirtualCaster() {
    this->byte_0x3c = 0;
    this->properties = new uint8_t[6]{};
}

// 528993 (client: 5176dc)
VirtualCaster::~VirtualCaster() {
    delete[] this->properties;
}

// 55d009 (client: 537860)
void VirtualCaster::Serialize(CArchive& ar) {
    Token::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->byte_0x3c;
        ar.Write(this->properties, 6); // WAT: writing 6 bytes of the pointer addresses, and not the data itself?
    } else {
        ar >> this->byte_0x3c;
        ar.Read(this->properties, 6);
    }
}
