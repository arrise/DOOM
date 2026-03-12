#include "doomdef.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"
#include "d_net.h"
#include "g_game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "../platform/platform_timer.h"

int mb_used = 16;
void I_Tactile(int on, int off, int total) {}

ticcmd_t emptycmd;
ticcmd_t* I_BaseTiccmd(void) { return &emptycmd; }

int I_GetHeapSize (void) { return mb_used*1024*1024; }

byte* I_ZoneBase (int* size) {
    *size = mb_used*1024*1024;
    return (byte *) malloc (*size);
}

int I_GetTime (void) {
    return (int)((Timer_GetMilliseconds() * TICRATE) / 1000);
}

void I_Init (void) {
    Timer_Init();
}

void I_Quit (void) {
    exit(0);
}

void I_WaitVBL(int count) {}
void I_BeginRead(void) {}
void I_EndRead(void) {}

byte* I_AllocLow(int length) {
    byte* mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}

extern void Window_ShowError(const char* error);

extern boolean demorecording;
void I_Error (char *error, ...) {
    va_list argptr;
    char buffer[2048];
    
    va_start (argptr,error);
    vsprintf (buffer, error, argptr);
    va_end (argptr);
    
    fprintf (stderr, "Error: %s\n", buffer);
    fflush(stderr);
    
#ifdef _WIN32
    Window_ShowError(buffer);
#endif
    
    exit(-1);
}
