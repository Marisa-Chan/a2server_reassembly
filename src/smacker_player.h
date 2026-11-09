#pragma once

// Smacker (.smk) video playback, reimplemented on top of libsmacker
// (src/lib/, LGPL 2.1). This replaces the retail binary's player, which was a
// Nival wrapper class around the RAD Smacker SDK (ctor 4c6e49, the family at
// 4c6e49-4c8xxx in a2serv6.exe / 4cb7e9ff.. in allods2.exe). The observable
// behavior is kept: pane rect + position, per-video .cfg fades and
// "panaraming" pane slides, the half-size rule for small videos, audio track
// playback through DirectSound (retail opened the smk with the SMACKTRACKS
// mask 0xff000 = all audio tracks, and SmackSoundUseDirectSound(0) let the
// SDK spin up its own DirectSound object) and the message-pump skip rules of
// the driver FUN_0049229c.

#include <stdint.h>
#include <windows.h>

#include "file.h"
#include "lib/smacker.h"

struct IDirectSoundBuffer;

// One in-flight video. Field/method comments give the binary counterpart.
class SmackerPlayer {
public:
    SmackerPlayer();  // 4c6e49
    ~SmackerPlayer(); // 4c6f60

    void SetHwnd(HWND hwnd);                          // 497510
    void SetPos(int32_t x, int32_t y);                // 4974e0
    bool Open(const char* path, int32_t pane_w, int32_t pane_h); // 4c77c2 (+ 4c733e)
    void GetVideoSize(int32_t* w, int32_t* h) const;  // 4c775a
    void SetPaneSize(int32_t r, int32_t b);           // 4c779d
    bool DoFrame();                                   // 4c716a(obj, 1)
    void Stop();                                      // 4c7d14

private:
    struct Fading {     // 16 bytes, like the binary's Fading entries
        int32_t startframe;
        int32_t endframe;
        float startfade;
        float endfade;
    };

    struct Panaraming { // 16 bytes, like the binary's Panaraming entries
        int32_t startframe;
        int32_t endframe;
        int32_t stepx;
        int32_t stepy;
    };

    void Close();       // 4c7556 (+ the wrapper free of 4c6db3, done in Stop/dtor)
    void RenderFrame(); // SmackDoFrame + SmackBlitSetPalette + SmackBlit + unlock/present
    void CreateAudioBuffer(uint8_t channels, uint8_t bitdepth, uint32_t rate);
    void WriteAudioChunk(const uint8_t* data, uint32_t size);

    smk video = nullptr; // [0x0] Smack handle
    HWND hwnd = nullptr; // [0x36c]

    // The .smk bytes, read out of the .res archive via File2. Retail handed
    // the File2 handle to SmackOpen (SMACKFILEHANDLE flag); libsmacker has no
    // CFile I/O, so we read the member into memory and keep it alive for the
    // decoder's lifetime (memory mode points into this buffer every frame).
    uint8_t* video_data = nullptr;
    uint32_t video_data_size = 0;

    // CPane borders (absolute rect coords) [0x348..0x354]
    int32_t pane_l = 0;
    int32_t pane_t = 0;
    int32_t pane_r = 0;
    int32_t pane_b = 0;

    int32_t pos_x = 0;  // [0x358]
    int32_t pos_y = 0;  // [0x35c]

    int32_t video_w = 0;   // from the smk header
    int32_t video_h = 0;
    uint32_t frame_count = 0;
    double usf = 0.0;      // microseconds per frame
    double next_frame_ms = 0.0;
    bool half_mode = false; // [0xda]

    int32_t fade_count = 0;  // [5]
    Fading* fades = nullptr; // [7]
    int32_t fade_cur = 0;    // [6]
    bool fade_active = false;   // [0x34] != 0
    float fade_factor = 0.0f;   // [0x33c]
    float fade_step = 0.0f;     // [0x340]
    int32_t fade_remaining = 0; // [0x344]
    uint8_t fade_palette[768];  // [0x38], palette scaled by the fade factor

    int32_t pan_count = 0;       // [8]
    Panaraming* pans = nullptr;  // [10]
    int32_t pan_cur = 0;         // [9]
    int32_t pan_stepx = 0;       // [0x360]
    int32_t pan_stepy = 0;       // [0x364]

    // Audio track 0, streamed into a ~2s DirectSound ring buffer on the
    // game's g_dsound object (the RAD SDK played through its own DirectSound
    // object instead; sharing the game's is equivalent here since music is
    // stopped while a video runs). One decoded chunk is appended per frame,
    // clamped to the free space ahead of the play cursor.
    IDirectSoundBuffer* audio_buffer = nullptr;
    uint32_t audio_buffer_size = 0;
    uint32_t audio_block_align = 0;
    uint32_t audio_write_pos = 0;
    bool audio_playing = false;
};

// Plays one .smk file fullscreen. Returns 1 when the video ran to its last
// frame, 0 when the user cancelled it (mouse button, key press or window
// close/quit - the driver's skip rules). Address: FUN_0049229c
// (client twin 496ae7, line-identical).
int32_t PlaySmackerVideo(HWND hwnd, const char* path);
