#include <winsock2.h>
#include "smacker_player.h"

#include <cstdio>
#include <cstring>
#include <timeapi.h>

#include "gfx.h"
#include "game_app.h"
#include "resource.h"
#include "sound.h"

SmackerPlayer::SmackerPlayer() {
    // 4c6e49
    memset(this->fade_palette, 0, sizeof(this->fade_palette));
}

SmackerPlayer::~SmackerPlayer() {
    // 4c6f60
    this->hwnd = nullptr;
    this->Stop();
    delete[] this->fades;
    delete[] this->pans;
    this->fades = nullptr;
    this->pans = nullptr;
    this->fade_count = 0;
    this->pan_count = 0;
}

void SmackerPlayer::SetHwnd(HWND hwnd) {
    // 497510. Only fed to the RAD SDK (palette focus bookkeeping); the
    // libsmacker renderer does not use it.
    this->hwnd = hwnd;
}

void SmackerPlayer::SetPos(int32_t x, int32_t y) {
    // 4974e0
    this->pos_x = x;
    this->pos_y = y;
}

void SmackerPlayer::SetPaneSize(int32_t r, int32_t b) {
    // 4c779d (writes the right/bottom borders)
    this->pane_r = r;
    this->pane_b = b;
}

void SmackerPlayer::GetVideoSize(int32_t* w, int32_t* h) const {
    // 4c775a
    if (this->video == nullptr) {
        *w = 0;
        *h = 0;
    } else {
        *w = this->video_w;
        *h = this->video_h;
    }
}

bool SmackerPlayer::Open(const char* path, int32_t pane_w, int32_t pane_h) {
    // 4c77c2 (+ 4c733e): read the per-video .cfg, open the smk, set the pane.
    this->Stop();

    int32_t startx = 0;
    int32_t starty = 0;

    // The config sits next to the video as a .reg sibling (the archive holds
    // e.g. "video\teleport\01.smk" and "video\teleport\01.reg"): everything
    // after the last '.' of the path is replaced by ".reg" (retail:
    // ReverseFind('.') + Left + ".reg" - bytes 634278 in a2serv6.exe).
    char cfg_path[268];
    strncpy(cfg_path, path, sizeof(cfg_path) - 8);
    cfg_path[sizeof(cfg_path) - 8] = 0;
    char* pdot = strrchr(cfg_path, '.');
    if (pdot != nullptr) {
        strcpy(pdot, ".reg");
    } else {
        strcat(cfg_path, ".reg");
    }

    // Retail shipped the .reg on the CD and let a missing one throw straight
    // out of the player (aborting the whole video sequence). Without a CD we
    // just fall back to the defaults (startx/starty 0, no fadings, no
    // panaramings).
    try {
        RegFile cfg(cfg_path);
        startx = cfg.GetInt("Common", "startx", 0);
        starty = cfg.GetInt("Common", "starty", 0);

        int32_t n_fadings = cfg.GetInt("Common", "nFadings", 0);
        delete[] this->fades;
        this->fades = nullptr;
        this->fade_count = 0;
        this->fade_cur = 0;
        if (n_fadings > 0) {
            this->fades = new Fading[n_fadings];
            this->fade_count = n_fadings;
            for (int32_t i = 0; i < n_fadings; i++) {
                char section[32];
                sprintf(section, "Fading%d", i + 1);
                this->fades[i].startframe = cfg.GetInt(section, "startframe", 0);
                this->fades[i].endframe = cfg.GetInt(section, "endframe", 0);
                this->fades[i].startfade = (float)cfg.GetDouble(section, "startfade", 0.0);
                this->fades[i].endfade = (float)cfg.GetDouble(section, "endfade", 0.0);
            }
        }

        int32_t n_pans = cfg.GetInt("Common", "nPanaramings", 0);
        delete[] this->pans;
        this->pans = nullptr;
        this->pan_count = 0;
        this->pan_cur = 0;
        if (n_pans > 0) {
            this->pans = new Panaraming[n_pans];
            this->pan_count = n_pans;
            for (int32_t i = 0; i < n_pans; i++) {
                char section[32];
                sprintf(section, "Panaraming%d", i + 1);
                this->pans[i].startframe = cfg.GetInt(section, "startframe", 0);
                this->pans[i].endframe = cfg.GetInt(section, "endframe", 0);
                this->pans[i].stepx = cfg.GetInt(section, "stepx", 0);
                this->pans[i].stepy = cfg.GetInt(section, "stepy", 0);
            }
        }
    } catch (...) {
        // no cfg (or unreadable) - keep the defaults
    }

    // Retail opened the .smk through File2 (which resolves the virtual
    // "video\..." path against the mounted .res archives) and passed the
    // handle to SmackOpen; we read the same bytes and decode from memory.
    File2 file;
    if (file.Open(path, 0, nullptr) == 0) {
        return false;
    }

    const uint32_t size = (uint32_t)file.GetLength();
    this->video_data = new uint8_t[size];
    if (size != 0) {
        file.Read(this->video_data, size);
    }
    file.Close();

    this->video_data_size = size;
    this->video = smk_open_memory(this->video_data, size);
    if (this->video == nullptr) {
        this->Close();
        return false;
    }

    // Retail passed SMACKTRACKS (0xff000) to SmackOpen - every audio track
    // decoded and played through DirectSound. The A2 videos carry track 0;
    // libsmacker only decodes it (no playback), so the chunks are streamed
    // into our own DirectSound buffer below. Enabling must happen before
    // smk_first, which renders (and decodes) the first frame.
    unsigned char track_mask = 0;
    unsigned char channels[7] = {0};
    unsigned char bitdepth[7] = {0};
    unsigned long audio_rate[7] = {0};
    smk_info_audio(this->video, &track_mask, channels, bitdepth, audio_rate);
    if ((track_mask & SMK_AUDIO_TRACK_0) != 0) {
        smk_enable_audio(this->video, 0, 1);
        this->CreateAudioBuffer(channels[0], bitdepth[0], audio_rate[0]);
    }

    if (smk_enable_video(this->video, 1) < 0 || smk_first(this->video) < 0) {
        this->Close();
        return false;
    }

    unsigned long w = 0;
    unsigned long h = 0;
    unsigned long frames = 0;
    double usf = 0.0;
    smk_info_video(this->video, &w, &h, nullptr);
    smk_info_all(this->video, nullptr, &frames, &usf);
    this->video_w = (int32_t)w;
    this->video_h = (int32_t)h;
    this->frame_count = (uint32_t)frames;
    this->usf = usf;
    this->next_frame_ms = (double)timeGetTime();

    // 4c733e: a video at most half the pane in both dimensions plays at its
    // native size (halved pane + halved position) instead of being scaled up.
    this->pane_r = pane_w;
    this->pane_b = pane_h;
    if (this->video_w * 2 <= this->pane_r && this->video_h * 2 <= this->pane_b) {
        this->pane_r >>= 1;
        this->pane_b >>= 1;
        this->pos_x /= 2;
        this->pos_y /= 2;
        this->half_mode = true;
    }

    // CPane::SetBorders(4c7fd0) - startx/starty from the cfg become the
    // left/top borders (retail overwrites l/t after the halving above).
    this->pane_l = startx;
    this->pane_t = starty;

    return true;
}

void SmackerPlayer::CreateAudioBuffer(uint8_t channels, uint8_t bitdepth, uint32_t rate) {
    if (g_dsound == nullptr || channels == 0 || rate == 0) {
        return;
    }

    if (bitdepth != 8 && bitdepth != 16) {
        return;
    }

    WAVEFORMATEX fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = channels;
    fmt.nSamplesPerSec = rate;
    fmt.wBitsPerSample = bitdepth;
    fmt.nBlockAlign = fmt.nChannels * bitdepth / 8;
    fmt.nAvgBytesPerSec = rate * fmt.nBlockAlign;

    // ~2 second ring (the first frame's chunk carries ~1s of lead audio and
    // must fit without clamping), rounded down to whole samples
    this->audio_block_align = fmt.nBlockAlign;
    this->audio_buffer_size = (fmt.nAvgBytesPerSec * 2) & ~(uint32_t)fmt.nBlockAlign;
    if (this->audio_buffer_size < fmt.nBlockAlign) {
        return;
    }

    DSBUFFERDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
    desc.dwBufferBytes = this->audio_buffer_size;
    desc.lpwfxFormat = &fmt;

    IDirectSoundBuffer* buffer = nullptr;
    if (FAILED(g_dsound->CreateSoundBuffer(&desc, &buffer, nullptr))) {
        this->audio_buffer_size = 0;
        return;
    }
    this->audio_buffer = buffer;
    this->audio_write_pos = 0;
    this->audio_playing = false;

    // DirectSound does not guarantee zeroed buffers; start silent so the
    // first frame's chunk is not preceded by noise.
    void* part1 = nullptr;
    void* part2 = nullptr;
    DWORD size1 = 0;
    DWORD size2 = 0;
    if (SUCCEEDED(buffer->Lock(0, 0, &part1, &size1, &part2, &size2, DSBLOCK_ENTIREBUFFER))) {
        if (part1 != nullptr) {
            memset(part1, 0, size1);
        }
        if (part2 != nullptr) {
            memset(part2, 0, size2);
        }
        buffer->Unlock(part1, size1, part2, size2);
    }
}

void SmackerPlayer::WriteAudioChunk(const uint8_t* data, uint32_t size) {
    if (this->audio_buffer == nullptr || data == nullptr || size == 0) {
        return;
    }

    DWORD play_pos = 0;
    DWORD write_cursor = 0;
    if (FAILED(this->audio_buffer->GetCurrentPosition(&play_pos, &write_cursor))) {
        return;
    }

    // Free space between the play cursor and our write position, keeping one
    // sample ahead of the cursor untouched. Computed as capacity minus used:
    // the naive (play - write) % size reports 0 both for an empty and a full
    // ring, which starved the very first write and left the buffer silent.
    // Late frames write only what fits; the tail of that chunk is dropped
    // (inaudible unless the system stalls).
    const uint32_t used_bytes = (this->audio_buffer_size + this->audio_write_pos - play_pos) % this->audio_buffer_size;
    uint32_t free_bytes = this->audio_buffer_size - used_bytes;
    if (free_bytes < size + this->audio_block_align) {
        size = free_bytes > this->audio_block_align ? free_bytes - this->audio_block_align : 0;
    }
    if (size == 0) {
        return;
    }

    void* part1 = nullptr;
    void* part2 = nullptr;
    DWORD size1 = 0;
    DWORD size2 = 0;
    if (FAILED(this->audio_buffer->Lock(this->audio_write_pos, size, &part1, &size1, &part2, &size2, 0))) {
        return;
    }

    if (part1 != nullptr) {
        memcpy(part1, data, size1);
    }
    if (part2 != nullptr) {
        memcpy(part2, data + size1, size2);
    }
    this->audio_buffer->Unlock(part1, size1, part2, size2);

    this->audio_write_pos = (this->audio_write_pos + size) % this->audio_buffer_size;

    if (!this->audio_playing) {
        this->audio_buffer->Play(0, 0, DSBPLAY_LOOPING);
        this->audio_playing = true;
    }
}

void SmackerPlayer::Close() {
    // 4c7556 (SmackClose + SmackSoundOnOff(smk, 0))
    if (this->audio_buffer != nullptr) {
        this->audio_buffer->Stop();
        this->audio_buffer->Release();
        this->audio_buffer = nullptr;
    }
    this->audio_playing = false;
    this->audio_write_pos = 0;

    if (this->video != nullptr) {
        smk_close(this->video);
        this->video = nullptr;
    }

    delete[] this->video_data;
    this->video_data = nullptr;
    this->video_data_size = 0;

    if (this->half_mode) {
        this->pane_l <<= 1;
        this->pane_t <<= 1;
        this->pane_r <<= 1;
        this->pane_b <<= 1;
        this->pos_x *= 2;
        this->pos_y *= 2;
        this->half_mode = false;
    }
}

void SmackerPlayer::Stop() {
    // 4c7d14 = close (4c7556) + drop the DirectDraw surface wrapper (4c6db3,
    // a no-op for libsmacker - we render through the shared backbuffer).
    this->Close();
}

void SmackerPlayer::RenderFrame() {
    // 4c70a1: while a fading is active, scale the frame palette by the fade
    // factor (factor accumulates per frame, char-truncated like the binary's
    // __ftol + char store).
    const uint8_t* pal = smk_get_palette(this->video);
    if (this->fade_active) {
        this->fade_factor += this->fade_step;
        for (int32_t i = 0; i < 768; i++) {
            this->fade_palette[i] = (uint8_t)(int32_t)(pal[i] * this->fade_factor);
        }
        this->fade_remaining--;
        if (this->fade_remaining == 0) {
            this->fade_active = false;
        }
        pal = this->fade_palette;
    }

    uint16_t pal16[256];
    for (int32_t i = 0; i < 256; i++) {
        pal16[i] = (uint16_t)GetColorRGB(pal[i * 3], pal[i * 3 + 1], pal[i * 3 + 2]);
    }

    const uint8_t* frame = smk_get_video(this->video);

    int32_t x0 = this->pos_x + this->pane_l;
    int32_t y0 = this->pos_y + this->pane_t;
    int32_t x1 = this->pos_x + this->pane_r;
    int32_t y1 = this->pos_y + this->pane_b;

    if (x0 < g_ScreenSize.left) {
        x0 = g_ScreenSize.left;
    }
    if (y0 < g_ScreenSize.top) {
        y0 = g_ScreenSize.top;
    }
    if (x1 > g_ScreenSize.right) {
        x1 = g_ScreenSize.right;
    }
    if (y1 > g_ScreenSize.bottom) {
        y1 = g_ScreenSize.bottom;
    }

    if (x1 <= x0 || y1 <= y0) {
        return;
    }

    const int32_t dw = x1 - x0;
    const int32_t dh = y1 - y0;

    LockSurface2();
    if (g_selDrawBitmap.lpSurface != nullptr) {
        // Retail delegated to the RAD SDK's SmackBlit (its own scaler into the
        // locked surface); we scale nearest-neighbor into the shared
        // backbuffer and present via the normal flush path.
        uint8_t* surf = (uint8_t*)g_selDrawBitmap.lpSurface;
        const int32_t pitch = g_selDrawBitmap.lPitch;

        for (int32_t dy = 0; dy < dh; dy++) {
            const uint8_t* src = frame + (dy * this->video_h / dh) * this->video_w;
            uint16_t* dst = (uint16_t*)(surf + (y0 + dy) * pitch + x0 * 2);
            for (int32_t dx = 0; dx < dw; dx++) {
                dst[dx] = pal16[src[dx * this->video_w / dw]];
            }
        }
    }
    UnlockSurface2();
    FlushScreen();
}

bool SmackerPlayer::DoFrame() {
    // 4c716a(obj, 1): block until the frame is due, apply triggers, render,
    // advance. Returns false after the last frame has been rendered.
    if (this->video == nullptr) {
        return false;
    }

    // 4c714d: wait out the frame time
    while ((double)timeGetTime() < this->next_frame_ms) {
        Sleep(1);
    }

    unsigned long cur = 0;
    unsigned long frames = 0;
    double usf = 0.0;
    smk_info_all(this->video, &cur, &frames, &usf);

    // 4c7d32: a fading starting on this frame
    if (this->fade_cur < this->fade_count && this->fades[this->fade_cur].startframe == (int32_t)cur) {
        const int32_t duration = this->fades[this->fade_cur].endframe - this->fades[this->fade_cur].startframe;
        this->fade_remaining = duration;
        this->fade_factor = this->fades[this->fade_cur].startfade;
        this->fade_step = (this->fades[this->fade_cur].endfade - this->fade_factor) / (float)duration;
        this->fade_active = true;
        this->fade_cur++;
    }

    // 4c7de9: a panaraming starting/ending on this frame
    if (this->pan_cur < this->pan_count) {
        if (this->pans[this->pan_cur].startframe == (int32_t)cur) {
            this->pan_stepx = this->pans[this->pan_cur].stepx;
            this->pan_stepy = this->pans[this->pan_cur].stepy;
        } else if (this->pans[this->pan_cur].endframe == (int32_t)cur) {
            this->pan_stepx = 0;
            this->pan_stepy = 0;
            this->pan_cur++;
        }
    }

    this->RenderFrame();

    // The audio chunk belongs to the current frame and is only valid until
    // smk_next advances, so append it right after rendering.
    if (this->audio_buffer != nullptr) {
        const unsigned char* chunk = smk_get_audio(this->video, 0);
        unsigned long chunk_size = smk_get_audio_size(this->video, 0);
        this->WriteAudioChunk(chunk, chunk_size);
    }

    // SmackDoFrame drew frame `cur`; done once it was the last one
    if (cur == frames - 1) {
        return false;
    }

    if (smk_next(this->video) < 0) {
        return false;
    }

    // 4c7ea8: slide the pane while a panaraming is active
    this->pane_l += this->pan_stepx;
    this->pane_t += this->pan_stepy;
    this->next_frame_ms += usf / 1000.0;
    return true;
}

int32_t PlaySmackerVideo(HWND hwnd, const char* path) {
    // 49229c (client: 496ae7, line-identical)
    SmackerPlayer player;
    player.SetHwnd(hwnd);
    player.SetPos(vis_scr_rect.left, vis_scr_rect.top + 0x3c);
    if (player.Open(path, 0x280, 0x168)) {
        int32_t w = 0;
        int32_t h = 0;
        player.GetVideoSize(&w, &h);
        if (h == 0x1e0) {
            // 480-line video: native size at the top of the screen
            player.SetPos(vis_scr_rect.left, vis_scr_rect.top);
            player.SetPaneSize(w, h);
        }

        MSG msg;
        for (;;) {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT || msg.message == WM_CLOSE ||
                    msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN ||
                    msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) {
                    if (msg.message == WM_QUIT || msg.message == WM_CLOSE) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    player.Stop();
                    return 0;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!player.DoFrame()) {
                break;
            }
        }
    }

    player.Stop();
    return 1;
}
