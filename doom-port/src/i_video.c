#include "doomdef.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "d_main.h"
#include "d_event.h"
#include "../platform/platform_window.h"

extern byte* screens[5];

// 32-bit pixel buffer for Window_Present
static unsigned int* g_screen_buffer_32;
// The 256 color palette converted to 32-bit (XRGB)
static unsigned int g_palette_32[256];

void I_PlatformEventCallback(platform_event_t* pev) {
    event_t ev;
    switch (pev->type) {
        case PLATFORM_EVENT_KEYDOWN:
            ev.type = ev_keydown;
            ev.data1 = pev->data1;
            D_PostEvent(&ev);
            break;
        case PLATFORM_EVENT_KEYUP:
            ev.type = ev_keyup;
            ev.data1 = pev->data1;
            D_PostEvent(&ev);
            break;
        case PLATFORM_EVENT_MOUSEMOVE:
            ev.type = ev_mouse;
            ev.data1 = pev->data1; // buttons
            ev.data2 = pev->data2; // x move
            ev.data3 = pev->data3; // y move
            D_PostEvent(&ev);
            break;
        case PLATFORM_EVENT_QUIT:
            I_Quit();
            break;
    }
}

void I_ShutdownGraphics(void) {
    Window_Shutdown();
    free(g_screen_buffer_32);
}

void I_StartFrame (void) {
    Window_ProcessEvents();
}

void I_GetEvent(void) {
    Window_ProcessEvents();
}

void I_StartTic (void) {
    Window_ProcessEvents();
}

void I_UpdateNoBlit (void) {}

void I_FinishUpdate (void) {
    // Map 8-bit paletted DOOM screen buffer to 32-bit window buffer
    byte* src = screens[0];
    unsigned int* dest = g_screen_buffer_32;
    int num_pixels = SCREENWIDTH * SCREENHEIGHT;
    
    if (src) {
        for (int i = 0; i < num_pixels; ++i) {
            dest[i] = g_palette_32[src[i]];
        }
        Window_Present(g_screen_buffer_32, SCREENWIDTH, SCREENHEIGHT);
    }
}

void I_ReadScreen (byte* scr) {
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette (byte* palette) {
    // DOOM palette is 256 colors, 3 bytes per color (RGB)
    for (int i = 0; i < 256; ++i) {
        byte r = gammatable[usegamma][*palette++];
        byte g = gammatable[usegamma][*palette++];
        byte b = gammatable[usegamma][*palette++];
        // Create XRGB standard Win32 32-bit pixel
        g_palette_32[i] = (r << 16) | (g << 8) | b;
    }
}

void I_InitGraphics(void) {
    screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);
    g_screen_buffer_32 = (unsigned int *) malloc (SCREENWIDTH * SCREENHEIGHT * 4);
    
    Window_Init(SCREENWIDTH, SCREENHEIGHT, "DOOM Windows Port");
    Window_SetEventCallback(I_PlatformEventCallback);
}
