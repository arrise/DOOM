#include "doomdef.h"
#include "i_sound.h"
#include "z_zone.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "../platform/platform_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_mixer.h>

#define NUM_CHANNELS    16      

// --- SOUND EFFECTS ---

void* I_GetSfx(sfxinfo_t* sfx, int* len) {
    unsigned char* raw_lump;
    int size, sfxlump;
    char name[20];

    sprintf(name, "ds%s", sfx->name);
    if (W_CheckNumForName(name) == -1) sfxlump = W_GetNumForName("dspistol");
    else sfxlump = W_GetNumForName(name);
    
    size = W_LumpLength(sfxlump);
    raw_lump = (unsigned char*)W_CacheLumpNum(sfxlump, PU_STATIC);
    
    // DOOM sfx are 8-bit unsigned, 11025Hz.
    // Skip 8-byte header (which has rate and sample count info but we know it's 11025).
    int sample_count = size - 8;
    uint8_t* pcm_data = raw_lump + 8;
    
    // Create a WAV header in memory to make it easy for SDL_mixer
    uint8_t* wav_buffer = (uint8_t*)malloc(sample_count + 44);
    memcpy(wav_buffer, "RIFF", 4);
    uint32_t riff_size = sample_count + 36; memcpy(wav_buffer + 4, &riff_size, 4);
    memcpy(wav_buffer + 8, "WAVEfmt ", 8);
    uint32_t fmt_size = 16; memcpy(wav_buffer + 16, &fmt_size, 4);
    uint16_t audio_format = 1; memcpy(wav_buffer + 20, &audio_format, 2); // PCM
    uint16_t num_channels = 1; memcpy(wav_buffer + 22, &num_channels, 2); // Mono
    uint32_t sample_rate = 11025; memcpy(wav_buffer + 24, &sample_rate, 4);
    uint32_t byte_rate = 11025; memcpy(wav_buffer + 28, &byte_rate, 4);
    uint16_t block_align = 1; memcpy(wav_buffer + 32, &block_align, 2);
    uint16_t bits_per_sample = 8; memcpy(wav_buffer + 34, &bits_per_sample, 2);
    memcpy(wav_buffer + 36, "data", 4);
    uint32_t data_size = sample_count; memcpy(wav_buffer + 40, &data_size, 4);
    memcpy(wav_buffer + 44, pcm_data, sample_count);
    
    SDL_RWops* rw = SDL_RWFromMem(wav_buffer, sample_count + 44);
    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1); // 1 = auto-close RWops
    
    // We keep wav_buffer alive because Mix_Chunk might point into it
    // In a real port we'd track these to free at shutdown.
    
    Z_Free(raw_lump);
    *len = sample_count;
    return (void *)chunk;
}

void I_SetChannels() {
    Mix_AllocateChannels(NUM_CHANNELS);
}

void I_SetSfxVolume(int volume) {
    snd_SfxVolume = volume;
    Mix_Volume(-1, (volume * 128) / 15);
}

void I_SetMusicVolume(int volume) {
    snd_MusicVolume = volume;
    // Scale music volume to a much lower range (0-64 instead of 0-128)
    // to give sound effects more presence in the mix.
    Mix_VolumeMusic((volume * 64) / 15);
}

int I_GetSfxLumpNum(sfxinfo_t* sfx) {
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority) {
    Mix_Chunk* chunk = (Mix_Chunk*)S_sfx[id].data;
    if (!chunk) return -1;
    
    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel != -1) {
        Mix_SetPanning(channel, 255 - sep, sep);
        Mix_Volume(channel, (vol * 128) / 15);
    }
    return channel;
}

void I_StopSound(int handle) {
    if (handle >= 0) Mix_HaltChannel(handle);
}

int I_SoundIsPlaying(int handle) {
    if (handle >= 0) return Mix_Playing(handle);
    return 0;
}

void I_UpdateSound(void) { }
void I_SubmitSound(void) { }
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch) {
    if (handle >= 0) {
        Mix_SetPanning(handle, 255 - sep, sep);
        Mix_Volume(handle, (vol * 128) / 15);
    }
}

void I_ShutdownSound(void) {
    Mix_CloseAudio();
    Mix_Quit();
}

void I_InitSound() {
    int i;
    for (i=1 ; i<NUMSFX ; i++) {
        if (!S_sfx[i].link) {
            int dummy_len;
            S_sfx[i].data = I_GetSfx(&S_sfx[i], &dummy_len);
        } else {
            S_sfx[i].data = S_sfx[i].link->data;
        }
    }
}

// --- MUSIC ---

typedef struct {
    char ID[4];
    uint16_t ScoreLen;
    uint16_t ScoreStart;
    uint16_t Channels;
    uint16_t SecChannels;
    uint16_t InstrCnt;
    uint16_t Dummy;
} mus_header_t;

static void WriteVarLen(SDL_RWops* rw, uint32_t value) {
    uint32_t buffer = value & 0x7F;
    while ((value >>= 7)) {
        buffer <<= 8;
        buffer |= ((value & 0x7F) | 0x80);
    }
    while (1) {
        uint8_t b = (uint8_t)(buffer & 0xFF);
        SDL_RWwrite(rw, &b, 1, 1);
        if (buffer & 0x80) buffer >>= 8;
        else break;
    }
}

static uint8_t* ConvertMusToMid(uint8_t* musdata, int* midisize) {
    mus_header_t* header = (mus_header_t*)musdata;
    if (memcmp(header->ID, "MUS\x1A", 4) != 0) return NULL;

    uint8_t* midi_temp = (uint8_t*)malloc(128 * 1024);
    SDL_RWops* rw = SDL_RWFromMem(midi_temp, 128 * 1024);
    if (!rw) { free(midi_temp); return NULL; }

    // MIDI Header
    SDL_RWwrite(rw, "MThd", 1, 4);
    uint32_t val32 = SDL_SwapBE32(6); SDL_RWwrite(rw, &val32, 4, 1);
    uint16_t val16 = SDL_SwapBE16(0); SDL_RWwrite(rw, &val16, 2, 1); // Format 0
    val16 = SDL_SwapBE16(1); SDL_RWwrite(rw, &val16, 2, 1); // 1 track
    val16 = SDL_SwapBE16(70); SDL_RWwrite(rw, &val16, 2, 1); // 70 ticks/quarter

    // Track Header
    SDL_RWwrite(rw, "MTrk", 1, 4);
    long size_patch_pos = (long)SDL_RWtell(rw);
    val32 = 0; SDL_RWwrite(rw, &val32, 4, 1);

    uint8_t* p = musdata + SDL_SwapLE16(header->ScoreStart);
    uint32_t delta = 0;
    uint8_t last_vel[16] = {0};
    int done = 0;

    while (!done) {
        uint8_t event = *p++;
        uint8_t channel = event & 0x0F;
        if (channel == 15) channel = 9; // DOOM percussion
        else if (channel >= 9) channel++;

        uint8_t type = (event >> 4) & 0x07;
        uint8_t last = event & 0x80;

        WriteVarLen(rw, delta);
        delta = 0;

        switch (type) {
            case 0: // Release note
                SDL_RWwrite(rw, (uint8_t[]){(uint8_t)(0x80 | channel), (uint8_t)(*p++ & 0x7F), 0}, 1, 3);
                break;
            case 1: { // Play note
                uint8_t note = *p++;
                if (note & 0x80) last_vel[channel] = *p++ & 0x7F;
                SDL_RWwrite(rw, (uint8_t[]){(uint8_t)(0x90 | channel), (uint8_t)(note & 0x7F), last_vel[channel]}, 1, 3);
                break;
            }
            case 2: // Pitch bend
                SDL_RWwrite(rw, (uint8_t[]){(uint8_t)(0xE0 | channel), 0, (uint8_t)(*p++ >> 1)}, 1, 3);
                break;
            case 3: // System event
                p++; // skip
                break;
            case 4: { // Controller
                uint8_t ctrl = *p++;
                uint8_t val = *p++;
                if (ctrl == 0) SDL_RWwrite(rw, (uint8_t[]){(uint8_t)(0xC0 | channel), val}, 1, 2);
                else SDL_RWwrite(rw, (uint8_t[]){(uint8_t)(0xB0 | channel), ctrl, val}, 1, 3);
                break;
            }
            case 6: // Score end
                done = 1;
                break;
        }
        if (last) {
            uint32_t wait = 0;
            do {
                uint8_t b = *p++;
                wait = (wait << 7) | (b & 0x7F);
                if (!(b & 0x80)) break;
            } while (1);
            delta = wait;
        }
    }

    // End of track
    WriteVarLen(rw, delta);
    SDL_RWwrite(rw, (uint8_t[]){0xFF, 0x2F, 0x00}, 1, 3);

    long track_end = (long)SDL_RWtell(rw);
    uint32_t track_size = (uint32_t)(track_end - size_patch_pos - 4);
    SDL_RWseek(rw, size_patch_pos, RW_SEEK_SET);
    val32 = SDL_SwapBE32(track_size);
    SDL_RWwrite(rw, &val32, 4, 1);

    *midisize = (int)track_end;
    SDL_RWclose(rw);
    return midi_temp;
}

static Mix_Music* music_table[64];

void I_InitMusic(void) {
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) SDL_InitSubSystem(SDL_INIT_AUDIO);
    Mix_Init(MIX_INIT_MID);
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0) {
        fprintf(stderr, "Mix_OpenAudio failed: %s\n", Mix_GetError());
    }
    for(int i=0; i<64; i++) music_table[i] = NULL;
}

void I_ShutdownMusic(void) { }

void I_PlaySong(int handle, int looping_param) {
    if (handle > 0 && handle <= 64 && music_table[handle-1]) {
        Mix_PlayMusic(music_table[handle-1], looping_param ? -1 : 0);
    }
}

void I_PauseSong(int handle) { Mix_PauseMusic(); }
void I_ResumeSong(int handle) { Mix_ResumeMusic(); }
void I_StopSong(int handle) { Mix_HaltMusic(); }

void I_UnRegisterSong(int handle) {
    if (handle > 0 && handle <= 64 && music_table[handle-1]) {
        Mix_FreeMusic(music_table[handle-1]);
        music_table[handle-1] = NULL;
    }
}

int I_RegisterSong(void* data) {
    int midisize = 0;
    uint8_t* mididata = ConvertMusToMid((uint8_t*)data, &midisize);
    if (!mididata) return 0;

    SDL_RWops* rw = SDL_RWFromMem(mididata, midisize);
    Mix_Music* music = Mix_LoadMUS_RW(rw, 1);
    if (!music) {
        fprintf(stderr, "Mix_LoadMUS_RW failed: %s\n", Mix_GetError());
        free(mididata);
        return 0;
    }
    
    for (int i=0; i<64; i++) {
        if (music_table[i] == NULL) {
            music_table[i] = music;
            return i + 1;
        }
    }
    return 0;
}

int I_QrySongPlaying(int handle) { return Mix_PlayingMusic(); }
