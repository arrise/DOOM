#ifndef PLATFORM_AUDIO_H
#define PLATFORM_AUDIO_H

#include <stdbool.h>
#include <stdint.h>

// The callback will be called by the audio device when it needs more data.
// It should fill 'stream' with 'len' bytes of data.
typedef void (*platform_audio_callback_t)(void* userdata, uint8_t* stream, int len);

// Initializes the audio subsystem.
// Requests a 16-bit signed stereo format at 'samplerate' Hz, with 'samples' per channel per callback.
bool Platform_InitAudio(int samplerate, int samples, platform_audio_callback_t callback, void* userdata);
void Platform_ShutdownAudio(void);

// For thread safety if the engine mixes in the main thread while the audio thread reads.
void Platform_LockAudio(void);
void Platform_UnlockAudio(void);

#endif // PLATFORM_AUDIO_H