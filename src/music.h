#ifndef MUSIC_H
#define MUSIC_H

#include "asm_mfc.h"
#include "mfc_templ.h"
#include "file.h"
#include <dsound.h>


class MusicPlayer : public CObject
{ 
public:
public:
	int32_t field_0x4;
	int32_t field_0x8;
	int32_t field_0xc;
	int32_t field_0x10;
	int32_t field_0x14;
	int32_t field_0x18;
	int32_t field_0x1c;
	int32_t field_0x20;
	int32_t field_0x24;
	int32_t field_0x28;
	CArray<int32_t> field_0x2c;
	CStringArray field_0x40;
	int32_t* field_0x54;
	int32_t field_0x58;
	int32_t field_0x5c;
	int32_t field_0x60;
	int32_t field_0x64;
	int32_t field_0x68;
	int32_t field_0x6c;
	int32_t field_0x70;
	int32_t field_0x74;
	int32_t field_0x78;
	int32_t field_0x7c;
	File2* field_0x80;
	uint8_t* field_0x84;
	WAVEFORMATEX format;
	IDirectSoundBuffer* ds_buffer;
};
ASSERT_SIZE(MusicPlayer, 0xa0);


#endif