#include "../platform_timer.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static LARGE_INTEGER g_frequency;
static LARGE_INTEGER g_start_time;

void Timer_Init(void) {
    QueryPerformanceFrequency(&g_frequency);
    QueryPerformanceCounter(&g_start_time);
}

uint64_t Timer_GetMilliseconds(void) {
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    
    uint64_t elapsed = current_time.QuadPart - g_start_time.QuadPart;
    return (elapsed * 1000) / g_frequency.QuadPart;
}
