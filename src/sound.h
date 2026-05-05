#pragma once

#include "asm_mfc.h"
#include "mfc_templ.h"
#include "dsound.h"

class SoundChannel;
class SfxSample;
struct SoundSettings;
struct MapMusicInfo;

extern IDirectSound* g_dsound; //65dd84
extern int32_t g_dsound_channel_num; //65ddd8
extern SoundChannel* g_dsound_channels; //65dda4
extern CArray<SfxSample*> g_SfxArray; //665eb0
extern SoundSettings g_SoundSettings; //660df0
extern CArray<MapMusicInfo*> g_mapmusicinfos; //660f00


class SfxSample : public CObject
{
public:


	void Play(int volume, int pan, int loop, uint8_t priority, int freq); //45c0cf
	void Release(); //45ba0f
public:
	uint32_t field_0x4;
	CString filename;
	int32_t loaded;
	IDirectSoundBuffer** buffers;
};
ASSERT_SIZE(SfxSample, 0x14);


class SoundChannel : public CObject
{
public:

	void Clear(); //41fdd0
public:
	uint8_t priority;
	uint8_t __pad[3];
	IDirectSoundBuffer* pbuffer;
	SfxSample* psample;
};
ASSERT_SIZE(SoundChannel, 0x10);


struct CSound
{
	SfxSample* sample;

	static void __cdecl Play(CSound &sound); //438ec0
};
ASSERT_SIZE(CSound, 4);


struct SoundSettings
{
	int32_t random;
	CStringArray* tracklist;
	int32_t mus_pos;
	int32_t field_0xc;
	int32_t sfx_pos;
	int32_t field_0x14;
	int32_t speech_pos;
	int32_t field_0x1c;
	int32_t field_0x20;
	int32_t music_enabled;

	SoundSettings(); //477860
	~SoundSettings(); //477920
};
ASSERT_SIZE(SoundSettings, 0x28);