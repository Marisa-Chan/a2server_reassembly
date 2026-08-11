#include "sound.h"
#include "main_window.h"
#include "file.h"

IDirectSound* g_dsound = nullptr; //65dd84
int32_t g_dsound_channel_num = 0; //65ddd8
SoundChannel* g_dsound_channels = nullptr; //65dda4

CArray<SfxSample*> g_SfxArray; //665eb0

SoundSettings g_SoundSettings; //660df0

CArray<MapMusicInfo*> g_mapmusicinfos; //660f00

MusicPlayer* MusicPlayer::Player = nullptr;
WAVEFORMATEX g_dsound_format;
DSBUFFERDESC g_dsound_buff_desc;

HWND g_dsound_hwnd;

int32_t g_CurrentMusicTrack = -1; //62faa0


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

void SoundChannel::Stop()
{ //41ff90
	if (pbuffer)
	{
		pbuffer->Stop();
		pbuffer->SetCurrentPosition(0);
	}
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

SoundChannel* SfxSample::FindPlayingChannel()
{ //45c004
	if (!g_dsound || !loaded)
		return nullptr;

	for (int i = 0; i < g_dsound_channel_num; i++)
	{
		DWORD status;
		buffers[i]->GetStatus(&status);
		if (status & DSBSTATUS_PLAYING)
		{
			for (int j = 0; j < g_dsound_channel_num; j++)
			{
				if (g_dsound_channels[j].pbuffer == buffers[i])
					return &g_dsound_channels[j];
			}
		}
	}
	return nullptr;
}



MusicPlayer::MusicPlayer(int32_t bufsz)
{ // 45b16d
	buffer_size = bufsz;
	file_opened = 0;

	if (!g_dsound)
	{
		ds_buffer = nullptr;
	}
	else
	{
		memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
		g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
		g_dsound_buff_desc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC);
		g_dsound_buff_desc.dwBufferBytes = buffer_size;
		g_dsound_buff_desc.lpwfxFormat = &g_dsound_format;

		if (g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, &ds_buffer, 0) == DS_OK)
		{
			format.wFormatTag = g_dsound_format.wFormatTag;
			format.nChannels = g_dsound_format.nChannels;
			format.nSamplesPerSec = g_dsound_format.nSamplesPerSec;
			format.nAvgBytesPerSec = g_dsound_format.nAvgBytesPerSec;
			format.nBlockAlign = g_dsound_format.nBlockAlign;
			format.wBitsPerSample = g_dsound_format.wBitsPerSample;
			format.cbSize = g_dsound_format.cbSize;
			update_disabled = 1;
			fadeout_enabled = 0;
			field_0x20 = 1;
			play_mode = 0;
			file_opened = 0;
			field_0x68 = 0;
			Player = this;
			samples = new uint8_t[buffer_size / 2];
			end_pos = buffer_size;
		}
		else
		{
			ds_buffer = nullptr;
		}
	}
}

MusicPlayer::~MusicPlayer()
{ // 45b321
	if (ds_buffer)
	{
		OnEndTrack();
		if (file_opened && file)
			delete file;
		ds_buffer->Release();
		delete[] samples;
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
			UnsetUpdateProc();
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

void MusicPlayer::UnsetUpdateProc()
{ //45a6ee
	((MainWindow*)AfxGetMainWnd())->SetMusicProc(nullptr);
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

void MusicPlayer::OpenTrack(int32_t track)
{ //45ae1d
	if (file_opened != 0)
	{
		if (file)
			delete file;
		file_opened = 0;
	}

	file = new File2(playlist[track], CFile::modeRead);
	file->Seek(0x14, 0);

	WAVEFORMATEX wav;
	memset(&wav, 0, sizeof(wav));

	if (file->Read(&wav, sizeof(wav)) == 0)
	{
		file_opened = 0;
		PostMessageA(g_dsound_hwnd, 0x486, 0, 0);
	}
	else
	{
		if (memcmp(&format, &wav, 0x10) != 0)
		{
			format.wFormatTag = wav.wFormatTag;
			format.nChannels = wav.nChannels;
			format.nSamplesPerSec = wav.nSamplesPerSec;
			format.nAvgBytesPerSec = wav.nAvgBytesPerSec;
			format.nBlockAlign = wav.nBlockAlign;
			format.wBitsPerSample = wav.wBitsPerSample;
			format.cbSize = wav.cbSize;

			ds_buffer->Release();

			memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
			g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
			g_dsound_buff_desc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC);
			g_dsound_buff_desc.dwBufferBytes = buffer_size;
			g_dsound_buff_desc.lpwfxFormat = &wav;

			g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, &ds_buffer, 0);
		}

		file->Seek(0x28, 0);
		file->Read(&data_length, 4);

		data_length = file->GetLength() - data_length;
		file_opened = 1;
		UpdateBufferTime();
	}
}

void MusicPlayer::SetPlayList(const CStringArray& plist)
{ //45b0ff
	if (ds_buffer)
	{
		OnEndTrack();
		playlist.Copy(plist);
		int sz = playlist.GetSize();
		field_0x2c.SetSize(sz);
		StartPlayTrack(rand() % sz);
	}
}

void MusicPlayer::Play()
{ //45b565
	if (g_SoundSettings.music_enabled != 0 && ds_buffer && file_opened)
	{
		UpdateBufferTime();
		if (update_disabled)
			SetUpdateProc();
		update_disabled = 0;
		ds_buffer->Play(0, 0, DSBPLAY_LOOPING);
	}
}

void MusicPlayer::UpdateBufferTime()
{ //45cb40
	int32_t tm = buffer_size / format.nAvgBytesPerSec * 1000 / 16;
	if (tm < 101)
		buffer_time = tm;
	else
		buffer_time = 100;
}

void MusicPlayer::SetUpdateProc()
{ //45a6c0
	MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();
	mwnd->SetMusicProc(MusicUpdaterProc);

	next_update_time = timeGetTime() + buffer_time;
}

void MusicPlayer::Update(uint32_t tm)
{ // 45a9b3
	if (next_update_time <= tm)
	{
		void* dbuf, * dbuf2;
		DWORD dbufsz, dbufsz2;

		next_update_time += buffer_time;

		DWORD play_cursor, write_cursor;
		ds_buffer->GetCurrentPosition(&play_cursor, &write_cursor);
		uint32_t sample_count = buffer_size / 2;

		elapsed_time += buffer_time;

		if (field_0x68 != 0)
			PostMessageA(g_dsound_hwnd, 0x467, current_track_id, GetElapsedSeconds());
		
		if (play_cursor < sample_count && use_buffer[1] != 0)
		{
			ReadPortion(samples);
			use_buffer[0] = 1;
			use_buffer[1] = 0;

			ds_buffer->GetCurrentPosition(&play_cursor, &write_cursor);
			if (play_cursor < sample_count)
			{
				ds_buffer->Lock(sample_count, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, samples, sample_count);
				ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);

				if (field_0x6c != 0)
					end_pos += sample_count;
			}
			else
			{
				ds_buffer->Lock(0, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, samples, sample_count);
				ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);
			}
		}

		if (play_cursor >= sample_count && use_buffer[0] != 0)
		{
			ReadPortion(samples);
			use_buffer[0] = 0;
			use_buffer[1] = 1;

			ds_buffer->GetCurrentPosition(&play_cursor, &write_cursor);
			if (play_cursor < sample_count)
			{
				ds_buffer->Lock(sample_count, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, samples, sample_count);
				ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);

				if (field_0x6c != 0)
					end_pos += sample_count;
			}
			else
			{
				ds_buffer->Lock(0, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, samples, sample_count);
				ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);
			}
		}

		field_0x6c = 0;

		if ((sample_count > play_cursor && sample_count > end_pos && play_cursor > end_pos) ||
			(sample_count < play_cursor && sample_count < end_pos && play_cursor > end_pos))
		{
			if (play_mode == 0)
				current_track_id = (current_track_id + 1) % playlist.GetSize();
			elapsed_time = 0;
			end_pos = buffer_size;
		}

		if (fadeout_enabled != 0)
		{
			LONG vol;
			ds_buffer->GetVolume(&vol);
			if (vol < fade_to_volume)
				OnEndTrack();
			else
				ds_buffer->SetVolume(ComputeVolumeFade(vol, buffer_time));
		}
	}
}

void MusicPlayer::MusicUpdaterProc()
{ //45a99c
	Player->Update(timeGetTime());
}

uint32_t MusicPlayer::GetElapsedSeconds()
{ //45cbe0
	if (!ds_buffer || file_opened == 0)
		return 0;
	return elapsed_time / 1000;
}

int32_t MusicPlayer::ComputeVolumeFade(int32_t cur_volume, uint32_t tm)
{ //45a8cc
	int32_t fade_val = fade_to_volume * tm / fade_length;
	int32_t vol_diff = ( ((pow(fade_to_volume / 4.0 + cur_volume, 4.0) * 0.3) / 
				          pow(fade_to_volume / 2.0, 4.0) + 0.9) *
				(((fade_to_volume + fade_start_volume) / (float)fade_to_volume) * (float)fade_val));
	return cur_volume - vol_diff;
}

void MusicPlayer::ReadPortion(uint8_t* buf)
{ //45a707
	int32_t max_fill_sz = buffer_size / 2;
	while (true)
	{
		uint32_t fpos = file->GetPosition();
		uint32_t remain = file->GetLength() - data_length;

		uint32_t rsz = max_fill_sz;

		if ((remain - fpos) < max_fill_sz)
			rsz = remain - fpos;

		if (file->Read(buf + (buffer_size / 2) - max_fill_sz, rsz) == 0)
		{
			file_opened = 0;
			PostMessageA(g_dsound_hwnd, 0x486, 0, 0);
			break;
		}

		max_fill_sz -= rsz;
		if (max_fill_sz == 0)
			break;

		end_pos = (buffer_size / 2) - max_fill_sz;
		field_0x6c = 1;

		if (g_CurrentMusicTrack < 0)
		{
			if (play_mode == 0)
				OpenTrack((current_track_id + 1) % playlist.GetSize());
			else
				OpenTrack(current_track_id);
		}
		else
			OpenTrack(g_CurrentMusicTrack);

		if (update_disabled == 0)
			UnsetUpdateProc();
		SetUpdateProc();
	}
}

void MusicPlayer::StartPlayTrack(int32_t track)
{ //45b6d0
	current_track_id = track;
	use_buffer[0] = 0;
	use_buffer[1] = 1;

	OpenTrack(current_track_id);
	ReadPortion(samples);

	void* dbuf, * dbuf2;
	DWORD dbufsz, dbufsz2;
	ds_buffer->Lock(0, buffer_size / 2, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
	memcpy(dbuf, samples, buffer_size / 2);
	ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);
	ds_buffer->SetCurrentPosition(0);

	elapsed_time = 0;
	end_pos = buffer_size;
}