#include "sound.h"

IDirectSound* g_dsound = nullptr; //65dd84
int32_t g_dsound_channel_num = 0; //65ddd8
SoundChannel* g_dsound_channels = nullptr; //65dda4

CArray<SfxSample*> g_SfxArray; //665eb0

SoundSettings g_SoundSettings; //660df0


SoundSettings::SoundSettings()
{
	//477860
	field_0xc = 5000;
	mus_pos = -700;
	field_0x14 = 5000;
	sfx_pos = -700;
	field_0x1c = 5000;
	speech_pos = -700;

	tracklist = new CStringArray();

	field_0x20 = 1;
	music_enabled = 1;
}

SoundSettings::~SoundSettings()
{
	//477920
	if (tracklist)
		delete tracklist;
}