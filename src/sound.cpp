#include "sound.h"
#include "main_window.h"

IDirectSound* g_dsound = nullptr; //65dd84
int32_t g_dsound_channel_num = 0; //65ddd8
SoundChannel* g_dsound_channels = nullptr; //65dda4

CArray<SfxSample*> g_SfxArray; //665eb0

SoundSettings g_SoundSettings; //660df0

CArray<MapMusicInfo*> g_mapmusicinfos; //660f00


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


void SoundChannel::Clear()
{ //41fdd0
	if (pbuffer)
		pbuffer->Stop();

	pbuffer = nullptr;
	priority = 0;
	psample = nullptr;
}


void SfxSample::Release()
{ //45ba0f
	if (loaded)
	{
		if (buffers)
		{
			for (int i = 0; i < g_dsound_channel_num; i++)
				buffers[i]->Release();

			delete[] buffers;

			buffers = nullptr;
		}
		loaded = 0;
	}
}


int MusicPlayer::GetState()
{ //451700
	if (!ds_buffer)
		return 5;

	if (!file_opened)
		return 4;
	
	if (update_disabled)
		return 3;

	if (fadeout_enabled)
		return 2;

	return 1;
}

void MusicPlayer::OnEndTrack()
{ //45b5cf
	if (ds_buffer && file_opened)
	{
		if (play_mode == 0)
		{
			DisableUpdate();
			update_disabled = 1;
			ds_buffer->Stop();
		}
		else
		{
			fadeout_enabled = 0;
			StartPlayTrack(track_to_play);
			SetVolume(g_SoundSettings.mus_pos);
			play_mode = 1;
		}
	}
}

void MusicPlayer::DisableUpdate()
{ //45a6ee
	((MainWindow*)AfxGetMainWnd())->FUN_0045cc80(nullptr);
}

void MusicPlayer::SetVolume(int32_t vol)
{ // 451790
	if (vol > 0)
		vol = 0;

	if (vol < -10000)
		vol = -10000;
	
	if (ds_buffer)
		ds_buffer->SetVolume(vol);
}