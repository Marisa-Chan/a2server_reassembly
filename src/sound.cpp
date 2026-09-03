#include "sound.h"
#include "main_window.h"
#include "file.h"
#include "resource.h"

IDirectSound* g_dsound = nullptr; //65dd84
IDirectSoundBuffer* g_dsound_buffer = nullptr; //65dd88
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


SfxBank SoundBank_fighter[2]; //6664b0
SfxBank SoundBank_mage[2]; //665dd8
SfxBank SoundBank_peasant[2]; //6663d8
SfxBank SoundBank_other[12]; //665ec8

WAVEFORMATEX g_SoundFmt; //65dd90


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

void SoundSettings::Load(HKEY& rkey)
{ //43d7c4
	DWORD sz = 4;
	RegQueryValueExA(rkey, "SoundRandom", nullptr, nullptr, (LPBYTE)&random, &sz);
	RegQueryValueExA(rkey, "SoundMusPos", nullptr, nullptr, (LPBYTE)&mus_pos, &sz);
	RegQueryValueExA(rkey, "SoundSfxPos", nullptr, nullptr, (LPBYTE)&sfx_pos, &sz);
	RegQueryValueExA(rkey, "SoundSpeechPos", nullptr, nullptr, (LPBYTE)&speech_pos, &sz);
	RegQueryValueExA(rkey, "MusicEnabled", nullptr, nullptr, (LPBYTE)&music_enabled, &sz);
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

SfxSample::~SfxSample()
{ //45b8c2
	if (g_dsound && loaded)
	{
		if (buffers)
		{
			for (int i = 0; i < g_dsound_channel_num; i++)
			{
				DWORD status;
				buffers[i]->GetStatus(&status);

				if (status & DSBSTATUS_PLAYING)
					buffers[i]->Stop();

				for (int j = 0; j < g_dsound_channel_num; j++)
				{
					if (g_dsound_channels[j].pbuffer == buffers[i])
						g_dsound_channels[j].pbuffer = nullptr;
				}
			}
		}
		Release();
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

void SfxSample::Play()
{
	SoundChannel* chnl = FindPlayingChannel();
	if (!chnl)
		Play(g_SoundSettings.sfx_pos, 0, 0, 0x80, 0);
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

		uint32_t data_length;
		file->Read(&data_length, 4);

		data_begin_pos = file->GetLength() - data_length;
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
	int32_t buffer_space = buffer_size / 2;
	while (true)
	{
		uint32_t fpos = file->GetPosition();
		uint32_t remain = file->GetLength() - data_begin_pos;

		uint32_t rsz = buffer_space;

		if ((remain - fpos) < buffer_space)  //ERROR? because it's compare filepos with only data chunk size
			rsz = remain - fpos;

		if (file->Read(buf + (buffer_size / 2) - buffer_space, rsz) == 0)
		{
			file_opened = 0;
			PostMessageA(g_dsound_hwnd, 0x486, 0, 0);
			break;
		}

		buffer_space -= rsz;
		if (buffer_space == 0)
			break;

		end_pos = (buffer_size / 2) - buffer_space;
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

void MusicPlayer::SetRandom(int random)
{ //45b655
	if (ds_buffer)
	{
		field_0x20 = random;
		play_mode = 0;
	}
}

int32_t MusicPlayer::GetVolume()
{ //451750
	LONG vol = 0;
	if (ds_buffer)
		ds_buffer->GetVolume(&vol);
	return vol;
}

int32_t MusicPlayer::GetBufPosition()
{ //45cba0
	DWORD pos = 0, wpos;
	if (ds_buffer)
		ds_buffer->GetCurrentPosition(&pos, &wpos);
	return pos;
}

void MusicPlayer::BeginFadeOut(int32_t len, int32_t vol)
{ //45b408
	if (ds_buffer && file_opened != 0)
	{
		fade_to_volume = vol;

		LONG svol;
		ds_buffer->GetVolume(&svol);
		fade_start_volume = svol;

		int32_t remain_bytes = (file->GetLength() - data_begin_pos) - file->GetPosition(); //Error because of remain!
		uint32_t ms_remain = remain_bytes * 1000 / format.nAvgBytesPerSec; //remain to the end in milliseconds

		uint32_t ms_buffend = 0xffffffff;
		if (end_pos != buffer_size)
		{
			int32_t bpos = GetBufPosition();
			if (end_pos < bpos)
				ms_buffend = (buffer_size - bpos) + end_pos;
			else
				ms_buffend = end_pos - bpos;

			//convert bytes count to milliseconds
			ms_buffend = ms_buffend * 1000 / format.nAvgBytesPerSec;
		}

		uint32_t ms_tofade = ms_remain;
		if (len < ms_remain)
			ms_tofade = len;

		if (ms_tofade < ms_buffend)
			fade_length = ms_tofade;
		else
			fade_length = ms_buffend;

		fadeout_enabled = 1;
	}
}

void SfxBank::Load(CString name)
{ //4c880e
	select_count = 0;
	attack_count = 0;
	move_count = 0;
	swarm_count = 0;

	File2 f;
	for (int i = 0; i < 4; i++)
	{
		CString fname;
		fname.Format("sfx\\characters\\%s\\select\\%d.wav", name, i + 1);
		select[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead))
		{
			f.Close();
			select_count++;
		}

		fname.Format("sfx\\characters\\%s\\attack\\%d.wav", name, i + 1);
		attack[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead))
		{
			f.Close();
			attack_count++;
		}

		fname.Format("sfx\\characters\\%s\\move\\%d.wav", name, i + 1);
		move[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead))
		{
			f.Close();
			move_count++;
		}

		fname.Format("sfx\\characters\\%s\\swarm\\%d.wav", name, i + 1);
		swarm[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead))
		{
			f.Close();
			swarm_count++;
		}
	}

	retreat = new SfxSample("sfx\\characters\\" + name + "\\retreat\\1.wav");
	defend = new SfxSample("sfx\\characters\\" + name + "\\defend\\1.wav");
	pickup = new SfxSample("sfx\\characters\\" + name + "\\pickup\\1.wav");
	easy = new SfxSample("sfx\\characters\\" + name + "\\easy\\1.wav");
	hard = new SfxSample("sfx\\characters\\" + name + "\\hard\\1.wav");
	dead = new SfxSample("sfx\\characters\\" + name + "\\dead\\1.wav");
}

void SfxBank::Unload()
{ //4c926b
	select_count = 0;
	attack_count = 0;
	move_count = 0;
	swarm_count = 0;

	for (int i = 0; i < 4; i++)
	{
		if (select[i])
			delete select[i];
		select[i] = nullptr;

		if (attack[i])
			delete attack[i];
		attack[i] = nullptr;

		if (move[i])
			delete move[i];
		move[i] = nullptr;

		if (swarm[i])
			delete swarm[i];
		swarm[i] = nullptr;
	}

	if (retreat)
		delete retreat;
	retreat = nullptr;
	if (defend)
		delete defend;
	defend = nullptr;
	if (pickup)
		delete pickup;
	pickup = nullptr;
	if (easy)
		delete easy;
	easy = nullptr;
	if (hard)
		delete hard;
	hard = nullptr;
	if (dead)
		delete dead;
	dead = nullptr;
}


int InitSound(HWND hwnd, int channels)
{ //45c489
	g_SoundFmt.wFormatTag = 1;
	g_SoundFmt.nChannels = 1;
	g_SoundFmt.nSamplesPerSec = 22050;
	g_SoundFmt.nBlockAlign = 2;
	g_SoundFmt.nAvgBytesPerSec = 44100;
	g_SoundFmt.wBitsPerSample = 16;
	g_SoundFmt.cbSize = 0;

	srand(timeGetTime());

	g_dsound_hwnd = hwnd;

	g_dsound_channels = nullptr;
	g_dsound_channel_num = channels;

	if (FAILED(DirectSoundCreate(nullptr, &g_dsound, 0)))
	{
		g_dsound = nullptr;
		return -1;
	}

	if (FAILED(g_dsound->SetCooperativeLevel(g_dsound_hwnd, DSSCL_EXCLUSIVE)))
	{
		FreeDSound();
		return -1;
	}

	g_dsound_channels = new SoundChannel[g_dsound_channel_num];

	memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
	g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
	g_dsound_buff_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	g_dsound_buff_desc.dwBufferBytes = 0;
	g_dsound_buff_desc.lpwfxFormat = nullptr;

	if (FAILED(g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, &g_dsound_buffer, 0)))
	{
		FreeDSound();
		return -1;
	}

	g_dsound_format.wFormatTag = 1;
	g_dsound_format.nChannels = 2;
	g_dsound_format.nSamplesPerSec = 22050;
	g_dsound_format.nBlockAlign = 4;
	g_dsound_format.nAvgBytesPerSec = 88200;
	g_dsound_format.wBitsPerSample = 16;
	g_dsound_format.cbSize = 0;

	if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format)))
	{
		g_dsound_format.wBitsPerSample /= 2;
		g_dsound_format.nAvgBytesPerSec /= 2;
		g_dsound_format.nBlockAlign /= 2;

		if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format)))
		{
			g_dsound_format.nChannels /= 2;
			g_dsound_format.nBlockAlign /= 2;
			g_dsound_format.nAvgBytesPerSec /= 2;

			if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format)))
			{
				g_dsound_format.nSamplesPerSec /= 2;
				g_dsound_format.nAvgBytesPerSec /= 2;
				if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format)))
				{
					FreeDSound();
					return -1;
				}
			}
		}
	}
	
	return 0;
}

void FreeDSound()
{ //45c7c5
	if (g_dsound)
		g_dsound->Release();
	g_dsound = nullptr;
	if (g_dsound_channels)
		delete[] g_dsound_channels;
	g_dsound_channels = nullptr;
}

void LoadGameSounds()
{ //4c8217
	RegFile res("sfx\\sfx.reg");

	int32_t sfx_count = res.GetInt("Global", "SfxCount", 0);

	g_SfxArray.SetSize(sfx_count + 1);

	CString tmp;
	for (int32_t i = 1; i <= sfx_count; i++)
	{
		tmp.Format("Sfx%d", i);
		char str[256];
		str[0] = '\0';
		res.GetSizedString("Sfx", tmp, "", str, sizeof(str));
		CString sname = str;
		if (!sname.IsEmpty())
		{
			CString fname = "sfx\\";
			fname += sname;
			fname += ".wav";
			g_SfxArray.SetAt(i, new SfxSample(fname));
		}
	}
	for (int i = 0; i < 2; i++)
	{
		static const char* names[] =
		{"m_fighter",
		 "f_fighter",
		"m_mage",
		"f_mage",
		"m_peasant",
		"f_peasant"};

		SoundBank_fighter[i].Load(names[i]);
		SoundBank_mage[i].Load(names[2 + i]);
		SoundBank_peasant[i].Load(names[4 + i]);
	}

	SoundBank_other[0].Load("aldor");
	SoundBank_other[1].Load("sagita");
	SoundBank_other[2].Load("hildarius");
	SoundBank_other[3].Load("galinel");
	SoundBank_other[4].Load("igles");
	SoundBank_other[5].Load("dina");
	SoundBank_other[6].Load("xbow");
	SoundBank_other[7].Load("knight");
	SoundBank_other[8].Load("druid");
	SoundBank_other[9].Load("troll");
	SoundBank_other[10].Load("orc");
	SoundBank_other[11].Load("mage");
}

