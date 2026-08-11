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
	SoundChannel* FindPlayingChannel(); //45c004
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
	void Stop(); //41ff90
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

class MusicPlayer : public CObject
{
public:
	static MusicPlayer *Player;
public:
	MusicPlayer(int32_t bufsz); //45b16d
	~MusicPlayer(); //45b321

	int GetState(); //451700
	void OnEndTrack(); //45b5cf
	void UnsetUpdateProc(); //45a6ee
	void SetVolume(int32_t vol); // 451790
	void StartPlayTrack(int32_t track); //45b6d0
	void OpenTrack(int32_t track); //45ae1d
	void SetPlayList(const CStringArray& plist); //45b0ff
	void Play(); //45b565
	void UpdateBufferTime(); //45cb40
	void SetUpdateProc(); //45a6c0
	void Update(uint32_t tm); //45a9b3

	uint32_t GetElapsedSeconds(); //45cbe0

	void ReadPortion(uint8_t* buf); //45a707

	int32_t ComputeVolumeFade(int32_t volume, uint32_t tm); //45a8cc

	static void MusicUpdaterProc(); //45a99c
public:
	int32_t gap_0x4;
	uint32_t next_update_time;
	int32_t update_disabled;
	int32_t file_opened;
	int32_t fadeout_enabled;
	int32_t play_mode;
	int32_t track_to_play; //in 1 mode
	int32_t field_0x20;
	int32_t use_buffer[2];
	CDWordArray field_0x2c;
	CStringArray playlist;
	int32_t buffer_size;
	int32_t current_track_id;
	int32_t buffer_time;
	int32_t fade_length;
	int32_t fade_to_volume;
	int32_t field_0x68;
	int32_t field_0x6c;
	int32_t elapsed_time;
	int32_t fade_start_volume;
	int32_t end_pos;
	int32_t data_length;
	CFile* file = nullptr;
	uint8_t* samples = nullptr;
	WAVEFORMATEX format;
	IDirectSoundBuffer* ds_buffer = nullptr;

};
ASSERT_SIZE(MusicPlayer, 0xa0);