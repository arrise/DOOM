#include "../platform_audio.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>

static bool g_audio_open = false;

bool Platform_InitAudio(int samplerate, int samples, platform_audio_callback_t callback, void* userdata) {
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = samplerate;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = samples;
    want.callback = (SDL_AudioCallback)callback;
    want.userdata = userdata;

    if (SDL_OpenAudio(&want, NULL) < 0) {
        fprintf(stderr, "SDL_OpenAudio failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_PauseAudio(0);
    g_audio_open = true;
    return true;
}

void Platform_ShutdownAudio(void) {
    if (g_audio_open) {
        SDL_CloseAudio();
        g_audio_open = false;
    }
}

void Platform_LockAudio(void) {
    if (g_audio_open) SDL_LockAudio();
}

void Platform_UnlockAudio(void) {
    if (g_audio_open) SDL_UnlockAudio();
}
