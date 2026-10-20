#include "sound.h"
#include "main_window.h"
#include "game_app.h"
#include "file.h"
#include "resource.h"
#include <mmreg.h>
#include <msacm.h>
#include <array>

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


SoundSettings::SoundSettings() {
	//477860
	this->field_0xc = 5000;
	this->mus_pos = -700;
	this->field_0x14 = 5000;
	this->sfx_pos = -700;
	this->field_0x1c = 5000;
	this->speech_pos = -700;

	this->tracklist = new CStringArray();

	this->music_allowed = 1;
	this->music_enabled = 1;
}

SoundSettings::~SoundSettings() {
	//477920
	if (this->tracklist) {
		delete this->tracklist;
	}
}

void SoundSettings::Load(HKEY& rkey) { //43d7c4
	DWORD sz = 4;
	RegQueryValueExA(rkey, "SoundRandom", nullptr, nullptr, (LPBYTE)&this->random, &sz);
	RegQueryValueExA(rkey, "SoundMusPos", nullptr, nullptr, (LPBYTE)&this->mus_pos, &sz);
	RegQueryValueExA(rkey, "SoundSfxPos", nullptr, nullptr, (LPBYTE)&this->sfx_pos, &sz);
	RegQueryValueExA(rkey, "SoundSpeechPos", nullptr, nullptr, (LPBYTE)&this->speech_pos, &sz);
	RegQueryValueExA(rkey, "MusicEnabled", nullptr, nullptr, (LPBYTE)&this->music_enabled, &sz);
}

// 43d877
void SoundSettings::Save(HKEY& rkey) {
	DWORD cbData = 4;

	RegSetValueExA(rkey, "SoundRandom", 0, REG_DWORD, (const BYTE*)&this->random, cbData);
	RegSetValueExA(rkey, "SoundMusPos", 0, REG_DWORD, (const BYTE*)&this->mus_pos, cbData);
	RegSetValueExA(rkey, "SoundSfxPos", 0, REG_DWORD, (const BYTE*)&this->sfx_pos, cbData);
	RegSetValueExA(rkey, "SoundSpeechPos", 0, REG_DWORD, (const BYTE*)&this->speech_pos, cbData);
	RegSetValueExA(rkey, "MusicEnabled", 0, REG_DWORD, (const BYTE*)&this->music_enabled, cbData);
}


void SoundChannel::Clear() { //41fdd0
	if (this->pbuffer) {
		this->pbuffer->Stop();
	}

	this->pbuffer = nullptr;
	this->priority = 0;
	this->psample = nullptr;
}

void SoundChannel::Stop() { //41ff90
	if (this->pbuffer) {
		this->pbuffer->Stop();
		this->pbuffer->SetCurrentPosition(0);
	}
}

void SoundChannel::Play(bool bLoop) { //45c408
	if (this->pbuffer) {
		this->pbuffer->Play(0, 0, bLoop ? DSBPLAY_LOOPING : 0);
	}
}

void SoundChannel::SetVolume(int32_t volume) { //41fea0
	if (volume < -10000) {
		volume = -10000;
	}

	if (volume > 0) {
		volume = 0;
	}

	if (this->pbuffer) {
		this->pbuffer->SetVolume(volume);
	}
}

void SoundChannel::SetPan(int32_t pan) { //41ff60
	if (this->pbuffer) {
		this->pbuffer->SetPan(pan);
	}
}



SoundChannel* __cdecl SoundChannel::GetChannel(int32_t priority) { //45c23e
	int i = 0;
	for (int i = 0; i < g_dsound_channel_num; i++) {
		SoundChannel& chn = g_dsound_channels[i];
		if (chn.pbuffer == nullptr) {
			chn.priority = 0;
			chn.psample = nullptr;
			return &chn;
		}

		DWORD st;
		chn.pbuffer->GetStatus(&st);

		if ((st & DSBSTATUS_PLAYING) == 0) {
			chn.pbuffer = nullptr;
			chn.priority = 0;
			chn.psample = nullptr;
			return &chn;
		}
	}

	int idx = -1;
	for (int i = 0; i < g_dsound_channel_num; i++) {
		if (g_dsound_channels[i].priority < priority) {
			priority = g_dsound_channels[i].priority;
			idx = i;
		}
	}

	if (idx == -1) {
		return nullptr;
	}

	SoundChannel& chn = g_dsound_channels[idx];

	chn.Stop();

	chn.pbuffer = nullptr;
	chn.priority = 0;
	chn.psample = nullptr;
	return &chn;
}

// 438dd0
void FreeSfxSample(SfxSample** sample) {
	if (*sample == nullptr) {
		return;
	}

	SoundChannel* chan = (*sample)->FindPlayingChannel();
	if (chan != nullptr) {
		chan->Stop();
		chan->pbuffer = nullptr;
	}

	delete *sample;
	*sample = nullptr;
}

// 438e40
void ReloadSfx(SfxSample** sample, const char* fname) {
	FreeSfxSample(sample);
	*sample = new SfxSample(fname);
}


SfxSample::~SfxSample() { //45b8c2
	if (g_dsound && loaded) {
		if (this->buffers) {
			for (int i = 0; i < g_dsound_channel_num; i++) {
				DWORD status;
				this->buffers[i]->GetStatus(&status);

				if (status & DSBSTATUS_PLAYING) {
					this->buffers[i]->Stop();
				}

				for (int j = 0; j < g_dsound_channel_num; j++) {
					if (g_dsound_channels[j].pbuffer == this->buffers[i]) {
						g_dsound_channels[j].pbuffer = nullptr;
					}
				}
			}
		}
		this->Release();
	}
}

void SfxSample::Release() { //45ba0f
	if (this->loaded) {
		if (this->buffers) {
			for (int i = 0; i < g_dsound_channel_num; i++) {
				this->buffers[i]->Release();
			}

			delete[] this->buffers;

			this->buffers = nullptr;
		}
		this->loaded = 0;
	}
}

SoundChannel* SfxSample::FindPlayingChannel() { //45c004
	if (!g_dsound || !this->loaded) {
		return nullptr;
	}

	for (int i = 0; i < g_dsound_channel_num; i++) {
		DWORD status;
		this->buffers[i]->GetStatus(&status);
		if (status & DSBSTATUS_PLAYING) {
			for (int j = 0; j < g_dsound_channel_num; j++) {
				if (g_dsound_channels[j].pbuffer == this->buffers[i]) {
					return &g_dsound_channels[j];
				}
			}
		}
	}
	return nullptr;
}

void SfxSample::Play() {
	SoundChannel* chnl = FindPlayingChannel();
	if (!chnl) {
		Play(g_SoundSettings.sfx_pos, 0, 0, 0x80, 0);
	}
}


void StopSfx(SfxSample** sample) { //438f20
	if (*sample != nullptr) {
		SoundChannel* chnl = (*sample)->FindPlayingChannel();
		if (chnl != nullptr) {
			chnl->Stop();
		}
	}
}

bool IsSfxPlaying(CSound* sound) { //475110
	if (sound->sample == nullptr) {
		return false;
	}
	return sound->sample->FindPlayingChannel() != nullptr;
}

void PlayAmbientSfx(SfxSample** sample) { //4a4740
	if (*sample != nullptr) {
		SoundChannel* chnl = (*sample)->FindPlayingChannel();
		if (chnl == nullptr) {
			(*sample)->Play(g_SoundSettings.sfx_pos, 0, 1, 0x80, 0);
		}
	}
}

void CSound::Play(CSound& sound) { //438ec0
	if (sound.sample != nullptr) {
		SoundChannel* chnl = sound.sample->FindPlayingChannel();
		if (chnl == nullptr) {
			sound.sample->Play(g_SoundSettings.sfx_pos, 0, 0, 0x80, 0);
		}
	}
}


void SfxSample::LoadSample() { //45ba94
	if (!g_dsound) {
		return;
	}

	File2 f;

	if (f.Open(this->filename, CFile::modeRead) == 0) {
		return;
	}

	this->buffers = new IDirectSoundBuffer*[g_dsound_channel_num];

	std::array<uint8_t, sizeof(ADPCMWAVEFORMAT) + sizeof(ADPCMCOEFSET) * 16> hdr;

	WAVEFORMATEX& pcm = *(WAVEFORMATEX*)hdr.data();
	ADPCMWAVEFORMAT& adpcm = *(ADPCMWAVEFORMAT*)hdr.data();

	f.Seek(0x14, CFile::begin);
	f.Read(hdr.data(), sizeof(WAVEFORMAT));


	if (pcm.wFormatTag == WAVE_FORMAT_PCM) {
		f.Read(&pcm.wBitsPerSample, 2);
	} else if (pcm.wFormatTag == WAVE_FORMAT_ADPCM) {
		f.Read(&adpcm.wfx.wBitsPerSample, 4);
		if (adpcm.wfx.cbSize != 0) {
			f.Read(&adpcm.wSamplesPerBlock, 4);
			if (adpcm.wNumCoef != 0) {
				f.Read(&adpcm.aCoef, adpcm.wNumCoef * sizeof(ADPCMCOEFSET));
			}
		}
	}

	uint32_t datsz;
	while (true) {
		uint32_t tmp;
		f.Read(&tmp, 4);

		f.Read(&datsz, 4);

		if (tmp == 0x61746164) {
			break;
		}

		f.Seek(datsz, CFile::current);
	}

	memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
	g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
	g_dsound_buff_desc.dwFlags = (DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME);
	g_dsound_buff_desc.dwBufferBytes = datsz;

	HACMSTREAM acmstream = nullptr;
	if (pcm.wFormatTag == WAVE_FORMAT_ADPCM) {
		g_dsound_buff_desc.lpwfxFormat = &g_SoundFmt;
		acmStreamOpen(&acmstream, 0, &adpcm.wfx, &g_SoundFmt, nullptr, 0, 0, 0);
		acmStreamSize(acmstream, datsz, &g_dsound_buff_desc.dwBufferBytes, 0);
	} else {
		g_dsound_buff_desc.lpwfxFormat = &pcm;
	}

	HRESULT res = g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, this->buffers, nullptr);
	if (res != 0) {
		memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
		g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
		g_dsound_buff_desc.dwFlags = (DSBCAPS_STATIC | DSBCAPS_LOCHARDWARE | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME); 0xe6;
		g_dsound_buff_desc.dwBufferBytes = datsz;
		g_dsound_buff_desc.lpwfxFormat = &pcm;

		res = g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, this->buffers, nullptr);
	}

	if (res == 0) {
		ACMSTREAMHEADER acm;
		void *bbuf1, *bbuf2;
		DWORD dwbuf1, dwbuf2;

		(*this->buffers)->Lock(0, datsz, &bbuf1, &dwbuf1, &bbuf2, &dwbuf2, 0);

		uint8_t* dbuf = (uint8_t*)bbuf1;
		if (pcm.wFormatTag == WAVE_FORMAT_ADPCM) {
			memset(&acm, 0, sizeof(ACMSTREAMHEADER));
			acm.cbStruct = sizeof(ACMSTREAMHEADER);
			acm.pbSrc = (LPBYTE)malloc(0x10000);
			acm.cbSrcLength = 0x10000;
			acmStreamSize(acmstream, 0x10000, &acm.cbDstLength, 0);

			acm.pbDst = (LPBYTE)malloc(acm.cbDstLength);
			acmStreamPrepareHeader(acmstream, &acm, 0);
		}

		int32_t nnum = datsz;
		while (nnum > 0x10000) {
			if (pcm.wFormatTag == WAVE_FORMAT_ADPCM) {
				f.Read(acm.pbSrc, 0x10000);
				acm.cbSrcLength = 0x10000;
				acmStreamConvert(acmstream, &acm, 4);
				memcpy(dbuf, acm.pbDst, acm.cbDstLengthUsed);
				dbuf += acm.cbDstLengthUsed;
			} else {
				f.Read(dbuf, 0x10000);
				dbuf += 0x10000;
			}

			nnum -= 0x10000;
			g_mousept.Update();
		}

		if (pcm.wFormatTag == WAVE_FORMAT_ADPCM) {
			f.Read(acm.pbSrc, nnum);
			acm.cbSrcLength = nnum;
			acmStreamConvert(acmstream, &acm, 0);

			memcpy(dbuf, acm.pbDst, acm.cbDstLengthUsed);
			acmStreamUnprepareHeader(acmstream, &acm, 0);

			free(acm.pbSrc);
			free(acm.pbDst);

			acmStreamClose(acmstream, 0);
		} else {
			f.Read(dbuf, nnum);
		}

		g_mousept.Update();

		(*this->buffers)->Unlock(bbuf1, dwbuf1, bbuf2, dwbuf2);

		for (int i = 1; i < g_dsound_channel_num; i++) {
			g_dsound->DuplicateSoundBuffer(*this->buffers, this->buffers + i);
		}

		this->loaded = 1;
	} else {
		acmStreamClose(acmstream, 0);
	}
}

void SfxSample::Play(int volume, int pan, int loop, uint8_t priority, int freq) { //45c0cf
	if (!g_dsound) {
		return;
	}

	if (loaded == 0) {
		LoadSample();
	}

	if (loaded && buffers) {
		if (volume < -10000) {
			volume = -10000;
		}

		for (int i = 0; i < g_dsound_channel_num; i++) {
			DWORD st;
			buffers[i]->GetStatus(&st);

			if ((st & DSBSTATUS_PLAYING) == 0) {
				SoundChannel* chan = SoundChannel::GetChannel(priority);
				if (chan) {
					chan->pbuffer = buffers[i];
					chan->psample = this;
					chan->priority = priority;

					buffers[i]->SetVolume(volume);
					buffers[i]->SetPan(pan);

					if (freq != 0) {
						buffers[i]->SetFrequency(freq);
					}

					chan->Play(loop);
				}
				break;
			}
		}
	}
}


MusicPlayer::MusicPlayer(int32_t bufsz) { // 45b16d
	this->buffer_size = bufsz;
	this->file_opened = 0;

	if (!g_dsound) {
		this->ds_buffer = nullptr;
	} else {
		memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
		g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
		g_dsound_buff_desc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC);
		g_dsound_buff_desc.dwBufferBytes = this->buffer_size;
		g_dsound_buff_desc.lpwfxFormat = &g_dsound_format;

		if (g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, &this->ds_buffer, 0) == DS_OK) {
			this->format.wFormatTag = g_dsound_format.wFormatTag;
			this->format.nChannels = g_dsound_format.nChannels;
			this->format.nSamplesPerSec = g_dsound_format.nSamplesPerSec;
			this->format.nAvgBytesPerSec = g_dsound_format.nAvgBytesPerSec;
			this->format.nBlockAlign = g_dsound_format.nBlockAlign;
			this->format.wBitsPerSample = g_dsound_format.wBitsPerSample;
			this->format.cbSize = g_dsound_format.cbSize;
			this->update_disabled = 1;
			this->fadeout_enabled = 0;
			this->field_0x20 = 1;
			this->play_mode = 0;
			this->file_opened = 0;
			this->play_notify = 0;
			Player = this;
			this->samples = new uint8_t[this->buffer_size / 2];
			this->end_pos = this->buffer_size;
		} else {
			this->ds_buffer = nullptr;
		}
	}
}

MusicPlayer::~MusicPlayer() { // 45b321
	if (this->ds_buffer) {
		this->OnEndTrack();
		if (this->file_opened && file) {
			delete file;
		}
		this->ds_buffer->Release();
		delete[] this->samples;
	}
}

int MusicPlayer::GetState() { //451700
	if (!this->ds_buffer) {
		return 5;
	}

	if (!this->file_opened) {
		return 4;
	}
	
	if (this->update_disabled) {
		return 3;
	}

	if (this->fadeout_enabled) {
		return 2;
	}

	return 1;
}

void MusicPlayer::OnEndTrack() { //45b5cf
	if (this->ds_buffer && file_opened) {
		if (this->play_mode == 0) {
			this->UnsetUpdateProc();
			this->update_disabled = 1;
			this->ds_buffer->Stop();
		} else {
			this->fadeout_enabled = 0;
			this->StartPlayTrack(this->track_to_play);
			this->SetVolume(g_SoundSettings.mus_pos);
			this->play_mode = 1;
		}
	}
}

void MusicPlayer::UnsetUpdateProc() { //45a6ee
	((MainWindow*)AfxGetMainWnd())->SetMusicProc(nullptr);
}

void MusicPlayer::SetVolume(int32_t vol) { // 451790
	if (vol > 0) {
		vol = 0;
	}

	if (vol < -10000) {
		vol = -10000;
	}
	
	if (this->ds_buffer) {
		this->ds_buffer->SetVolume(vol);
	}
}

void MusicPlayer::OpenTrack(int32_t track) { //45ae1d
	if (this->file_opened != 0) {
		if (this->file) {
			delete this->file;
		}
		this->file_opened = 0;
	}

	this->file = new File2(this->playlist[track], CFile::modeRead);
	this->file->Seek(0x14, 0);

	WAVEFORMATEX wav;
	memset(&wav, 0, sizeof(wav));

	if (this->file->Read(&wav, sizeof(wav)) == 0) {
		this->file_opened = 0;
		PostMessageA(g_dsound_hwnd, 0x486, 0, 0);
	} else {
		if (memcmp(&this->format, &wav, 0x10) != 0) {
			this->format.wFormatTag = wav.wFormatTag;
			this->format.nChannels = wav.nChannels;
			this->format.nSamplesPerSec = wav.nSamplesPerSec;
			this->format.nAvgBytesPerSec = wav.nAvgBytesPerSec;
			this->format.nBlockAlign = wav.nBlockAlign;
			this->format.wBitsPerSample = wav.wBitsPerSample;
			this->format.cbSize = wav.cbSize;

			this->ds_buffer->Release();

			memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
			g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
			g_dsound_buff_desc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC);
			g_dsound_buff_desc.dwBufferBytes = this->buffer_size;
			g_dsound_buff_desc.lpwfxFormat = &wav;

			g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, &this->ds_buffer, 0);
		}

		this->file->Seek(0x28, 0);

		uint32_t data_length;
		this->file->Read(&data_length, 4);

		this->data_begin_pos = this->file->GetLength() - data_length;
		this->file_opened = 1;
		this->UpdateBufferTime();
	}
}

void MusicPlayer::SetPlayList(const CStringArray& plist) { //45b0ff
	if (this->ds_buffer) {
		this->OnEndTrack();
		this->playlist.Copy(plist);
		int sz = this->playlist.GetSize();
		this->track_ids.SetSize(sz);
		this->StartPlayTrack(rand() % sz);
	}
}

void MusicPlayer::Play() { //45b565
	if (g_SoundSettings.music_enabled != 0 && this->ds_buffer && file_opened) {
		this->UpdateBufferTime();
		if (this->update_disabled) {
			this->SetUpdateProc();
		}
		this->update_disabled = 0;
		this->ds_buffer->Play(0, 0, DSBPLAY_LOOPING);
	}
}

void MusicPlayer::UpdateBufferTime() { //45cb40
	int32_t tm = this->buffer_size / this->format.nAvgBytesPerSec * 1000 / 16;
	if (tm < 101) {
		this->buffer_time = tm;
	} else {
		this->buffer_time = 100;
	}
}

void MusicPlayer::SetUpdateProc() { //45a6c0
	MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();
	mwnd->SetMusicProc(MusicUpdaterProc);

	this->next_update_time = timeGetTime() + this->buffer_time;
}

void MusicPlayer::Update(uint32_t tm) { // 45a9b3
	if (this->next_update_time <= tm) {
		void* dbuf, * dbuf2;
		DWORD dbufsz, dbufsz2;

		this->next_update_time += this->buffer_time;

		DWORD play_cursor, write_cursor;
		this->ds_buffer->GetCurrentPosition(&play_cursor, &write_cursor);
		uint32_t sample_count = this->buffer_size / 2;

		this->elapsed_time += this->buffer_time;

		if (this->play_notify != 0) {
			PostMessageA(g_dsound_hwnd, 0x467, this->current_track_id, this->GetElapsedSeconds());
		}
		
		if (play_cursor < sample_count && use_buffer[1] != 0) {
			this->ReadPortion(this->samples);
			use_buffer[0] = 1;
			use_buffer[1] = 0;

			this->ds_buffer->GetCurrentPosition(&play_cursor, &write_cursor);
			if (play_cursor < sample_count) {
				this->ds_buffer->Lock(sample_count, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, this->samples, sample_count);
				this->ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);

				if (this->track_wrap_pending != 0) {
					this->end_pos += sample_count;
				}
			} else {
				this->ds_buffer->Lock(0, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, this->samples, sample_count);
				this->ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);
			}
		}

		if (play_cursor >= sample_count && use_buffer[0] != 0) {
			this->ReadPortion(this->samples);
			use_buffer[0] = 0;
			use_buffer[1] = 1;

			this->ds_buffer->GetCurrentPosition(&play_cursor, &write_cursor);
			if (play_cursor < sample_count) {
				this->ds_buffer->Lock(sample_count, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, this->samples, sample_count);
				this->ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);

				if (this->track_wrap_pending != 0) {
					this->end_pos += sample_count;
				}
			} else {
				this->ds_buffer->Lock(0, sample_count, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
				memcpy(dbuf, this->samples, sample_count);
				this->ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);
			}
		}

		this->track_wrap_pending = 0;

		if ((sample_count > play_cursor && sample_count > this->end_pos && play_cursor > this->end_pos) ||
			(sample_count < play_cursor && sample_count < this->end_pos && play_cursor > this->end_pos)) {
			if (this->play_mode == 0) {
				this->current_track_id = (this->current_track_id + 1) % this->playlist.GetSize();
			}
			this->elapsed_time = 0;
			this->end_pos = this->buffer_size;
		}

		if (this->fadeout_enabled != 0) {
			LONG vol;
			this->ds_buffer->GetVolume(&vol);
			if (vol < this->fade_to_volume) {
				this->OnEndTrack();
			} else {
				this->ds_buffer->SetVolume(this->ComputeVolumeFade(vol, this->buffer_time));
			}
		}
	}
}

void MusicPlayer::MusicUpdaterProc() { //45a99c
	Player->Update(timeGetTime());
}

uint32_t MusicPlayer::GetElapsedSeconds() { //45cbe0
	if (!this->ds_buffer || this->file_opened == 0) {
		return 0;
	}
	return this->elapsed_time / 1000;
}

int32_t MusicPlayer::ComputeVolumeFade(int32_t cur_volume, uint32_t tm) { //45a8cc
	int32_t fade_val = this->fade_to_volume * tm / this->fade_length;
	int32_t vol_diff = ( ((pow(this->fade_to_volume / 4.0 + cur_volume, 4.0) * 0.3) / 
				          pow(this->fade_to_volume / 2.0, 4.0) + 0.9) *
				(((this->fade_to_volume + this->fade_start_volume) / (float)this->fade_to_volume) * (float)fade_val));
	return cur_volume - vol_diff;
}

void MusicPlayer::ReadPortion(uint8_t* buf) { //45a707
	int32_t buffer_space = this->buffer_size / 2;
	while (true) {
		uint32_t fpos = this->file->GetPosition();
		uint32_t remain = this->file->GetLength() - this->data_begin_pos;

		uint32_t rsz = buffer_space;

		if ((remain - fpos) < buffer_space) {  //ERROR? because it's compare filepos with only data chunk size
			rsz = remain - fpos;
		}

		if (this->file->Read(buf + (this->buffer_size / 2) - buffer_space, rsz) == 0) {
			this->file_opened = 0;
			PostMessageA(g_dsound_hwnd, 0x486, 0, 0);
			break;
		}

		buffer_space -= rsz;
		if (buffer_space == 0) {
			break;
		}

		this->end_pos = (this->buffer_size / 2) - buffer_space;
		this->track_wrap_pending = 1;

		if (g_CurrentMusicTrack < 0) {
			if (this->play_mode == 0) {
				this->OpenTrack((this->current_track_id + 1) % this->playlist.GetSize());
			} else {
				this->OpenTrack(this->current_track_id);
			}
		} else {
			this->OpenTrack(g_CurrentMusicTrack);
		}

		if (this->update_disabled == 0) {
			this->UnsetUpdateProc();
		}
		this->SetUpdateProc();
	}
}

void MusicPlayer::StartPlayTrack(int32_t track) { //45b6d0
	this->current_track_id = track;
	this->use_buffer[0] = 0;
	this->use_buffer[1] = 1;

	this->OpenTrack(this->current_track_id);
	this->ReadPortion(this->samples);

	void* dbuf, * dbuf2;
	DWORD dbufsz, dbufsz2;
	this->ds_buffer->Lock(0, this->buffer_size / 2, &dbuf, &dbufsz, &dbuf2, &dbufsz2, 0);
	memcpy(dbuf, this->samples, this->buffer_size / 2);
	this->ds_buffer->Unlock(dbuf, dbufsz, dbuf2, dbufsz2);
	this->ds_buffer->SetCurrentPosition(0);

	this->elapsed_time = 0;
	this->end_pos = this->buffer_size;
}

void MusicPlayer::SetRandom(int random) { //45b655
	if (this->ds_buffer) {
		this->field_0x20 = random;
		this->play_mode = 0;
	}
}

int32_t MusicPlayer::GetVolume() { //451750
	LONG vol = 0;
	if (this->ds_buffer) {
		this->ds_buffer->GetVolume(&vol);
	}
	return vol;
}

int32_t MusicPlayer::GetBufPosition() { //45cba0
	DWORD pos = 0, wpos;
	if (this->ds_buffer) {
		this->ds_buffer->GetCurrentPosition(&pos, &wpos);
	}
	return pos;
}

void MusicPlayer::BeginFadeOut(int32_t len, int32_t vol) { //45b408
	if (this->ds_buffer && file_opened != 0) {
		this->fade_to_volume = vol;

		LONG svol;
		this->ds_buffer->GetVolume(&svol);
		this->fade_start_volume = svol;

		int32_t remain_bytes = (this->file->GetLength() - this->data_begin_pos) - this->file->GetPosition(); //Error because of remain!
		uint32_t ms_remain = remain_bytes * 1000 / this->format.nAvgBytesPerSec; //remain to the end in milliseconds

		uint32_t ms_buffend = 0xffffffff;
		if (this->end_pos != this->buffer_size) {
			int32_t bpos = this->GetBufPosition();
			if (this->end_pos < bpos) {
				ms_buffend = (this->buffer_size - bpos) + this->end_pos;
			} else {
				ms_buffend = this->end_pos - bpos;

			//convert bytes count to milliseconds
			}
			ms_buffend = ms_buffend * 1000 / this->format.nAvgBytesPerSec;
		}

		uint32_t ms_tofade = ms_remain;
		if (len < ms_remain) {
			ms_tofade = len;
		}

		if (ms_tofade < ms_buffend) {
			this->fade_length = ms_tofade;
		} else {
			this->fade_length = ms_buffend;
		}

		this->fadeout_enabled = 1;
	}
}

void MusicPlayer::FUN_0045b681(int32_t track) { //45b681
	this->play_mode = 1;
	this->BeginFadeOut(2000, 8000);
	this->track_to_play = track;
}

void SfxBank::Load(CString name) { //4c880e
	this->select_count = 0;
	this->attack_count = 0;
	this->move_count = 0;
	this->swarm_count = 0;

	File2 f;
	for (int i = 0; i < 4; i++) {
		CString fname;
		fname.Format("sfx\\characters\\%s\\select\\%d.wav", name, i + 1);
		this->select[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead)) {
			f.Close();
			this->select_count++;
		}

		fname.Format("sfx\\characters\\%s\\attack\\%d.wav", name, i + 1);
		this->attack[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead)) {
			f.Close();
			this->attack_count++;
		}

		fname.Format("sfx\\characters\\%s\\move\\%d.wav", name, i + 1);
		this->move[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead)) {
			f.Close();
			this->move_count++;
		}

		fname.Format("sfx\\characters\\%s\\swarm\\%d.wav", name, i + 1);
		this->swarm[i] = new SfxSample(fname);
		if (f.Open(fname, CFile::modeRead)) {
			f.Close();
			this->swarm_count++;
		}
	}

	this->retreat = new SfxSample("sfx\\characters\\" + name + "\\retreat\\1.wav");
	this->defend = new SfxSample("sfx\\characters\\" + name + "\\defend\\1.wav");
	this->pickup = new SfxSample("sfx\\characters\\" + name + "\\pickup\\1.wav");
	this->easy = new SfxSample("sfx\\characters\\" + name + "\\easy\\1.wav");
	this->hard = new SfxSample("sfx\\characters\\" + name + "\\hard\\1.wav");
	this->dead = new SfxSample("sfx\\characters\\" + name + "\\dead\\1.wav");
}

void SfxBank::Unload() { //4c926b
	this->select_count = 0;
	this->attack_count = 0;
	this->move_count = 0;
	this->swarm_count = 0;

	for (int i = 0; i < 4; i++) {
		if (this->select[i]) {
			delete this->select[i];
		}
		this->select[i] = nullptr;

		if (this->attack[i]) {
			delete this->attack[i];
		}
		this->attack[i] = nullptr;

		if (this->move[i]) {
			delete this->move[i];
		}
		this->move[i] = nullptr;

		if (this->swarm[i]) {
			delete this->swarm[i];
		}
		this->swarm[i] = nullptr;
	}

	if (this->retreat) {
		delete this->retreat;
	}
	this->retreat = nullptr;
	if (this->defend) {
		delete this->defend;
	}
	this->defend = nullptr;
	if (this->pickup) {
		delete this->pickup;
	}
	this->pickup = nullptr;
	if (this->easy) {
		delete this->easy;
	}
	this->easy = nullptr;
	if (this->hard) {
		delete this->hard;
	}
	this->hard = nullptr;
	if (this->dead) {
		delete this->dead;
	}
	this->dead = nullptr;
}


int InitSound(HWND hwnd, int channels) { //45c489
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

	if (FAILED(DirectSoundCreate(nullptr, &g_dsound, 0))) {
		g_dsound = nullptr;
		return -1;
	}

	if (FAILED(g_dsound->SetCooperativeLevel(g_dsound_hwnd, DSSCL_EXCLUSIVE))) {
		FreeDSound();
		return -1;
	}

	g_dsound_channels = new SoundChannel[g_dsound_channel_num];

	memset(&g_dsound_buff_desc, 0, sizeof(g_dsound_buff_desc));
	g_dsound_buff_desc.dwSize = sizeof(g_dsound_buff_desc);
	g_dsound_buff_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	g_dsound_buff_desc.dwBufferBytes = 0;
	g_dsound_buff_desc.lpwfxFormat = nullptr;

	if (FAILED(g_dsound->CreateSoundBuffer(&g_dsound_buff_desc, &g_dsound_buffer, 0))) {
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

	if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format))) {
		g_dsound_format.wBitsPerSample /= 2;
		g_dsound_format.nAvgBytesPerSec /= 2;
		g_dsound_format.nBlockAlign /= 2;

		if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format))) {
			g_dsound_format.nChannels /= 2;
			g_dsound_format.nBlockAlign /= 2;
			g_dsound_format.nAvgBytesPerSec /= 2;

			if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format))) {
				g_dsound_format.nSamplesPerSec /= 2;
				g_dsound_format.nAvgBytesPerSec /= 2;
				if (FAILED(g_dsound_buffer->SetFormat(&g_dsound_format))) {
					FreeDSound();
					return -1;
				}
			}
		}
	}
	
	return 0;
}

void FreeDSound() { //45c7c5
	if (g_dsound) {
		g_dsound->Release();
	}
	g_dsound = nullptr;
	if (g_dsound_channels) {
		delete[] g_dsound_channels;
	}
	g_dsound_channels = nullptr;
}

void LoadGameSounds() { //4c8217
	RegFile res("sfx\\sfx.reg");

	int32_t sfx_count = res.GetInt("Global", "SfxCount", 0);

	g_SfxArray.SetSize(sfx_count + 1);

	CString tmp;
	for (int32_t i = 1; i <= sfx_count; i++) {
		tmp.Format("Sfx%d", i);
		char str[256];
		str[0] = '\0';
		res.GetSizedString("Sfx", tmp, "", str, sizeof(str));
		CString sname = str;
		if (!sname.IsEmpty()) {
			CString fname = "sfx\\";
			fname += sname;
			fname += ".wav";
			g_SfxArray.SetAt(i, new SfxSample(fname));
		}
	}
	for (int i = 0; i < 2; i++) {
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

// 4C86F5
void UnloadAllSfx() {
	for (int32_t i = 0; i < g_SfxArray.GetSize(); i++) {
		SfxSample* sfx = g_SfxArray[i];
		if (sfx != nullptr) {
			delete sfx;
		}
	}
	g_SfxArray.RemoveAll();

	for (int32_t i = 0; i < 2; i++) {
		SoundBank_fighter[i].Unload();
		SoundBank_mage[i].Unload();
		SoundBank_peasant[i].Unload();
	}
	for (int32_t i = 0; i < 12; i++) {
		SoundBank_other[i].Unload();
	}
}

// 476c0d
void UpdatePositionalMusic(int32_t x, int32_t y) {
	int32_t best = -1;
	double best_dist = 1e20;

	for (int32_t i = 0; i < g_mapmusicinfos.GetSize(); i++) {
		MapMusicInfo* info = g_mapmusicinfos[i];
		if (info->x == 0 && info->y == 0) {
			if (best_dist > 1e15) {
				best = i;
				best_dist = 1e10;
			}
		} else {
			if (info->melody_type_id[0] != 0xFFFFFFFF || info->melody_type_id[1] != 0xFFFFFFFF ||
				info->melody_type_id[2] != 0xFFFFFFFF || info->melody_type_id[3] != 0xFFFFFFFF) {
				double dx = (double)(int32_t)(x - (info->x << 8));
				double dy = (double)(int32_t)(y - (info->y << 8));
				double dist = sqrt(dx * dx + dy * dy);
				if (dist < (double)(int32_t)(info->radius << 8) && dist < best_dist) {
					best_dist = dist;
					best = i;
				}
			}
		}
	}

	if (best >= 0) {
		do {
			MapMusicInfo* info = g_mapmusicinfos[best];
			g_CurrentMusicTrack = info->melody_type_id[Random0N(3)];
		} while (g_CurrentMusicTrack == -1);
	}
}

