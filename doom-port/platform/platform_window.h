#pragma once

#include <stdint.h>

typedef enum {
    PLATFORM_EVENT_KEYDOWN,
    PLATFORM_EVENT_KEYUP,
    PLATFORM_EVENT_MOUSEMOVE,
    PLATFORM_EVENT_QUIT
} platform_event_type_t;

typedef struct {
    platform_event_type_t type;
    int data1;
    int data2;
    int data3;
} platform_event_t;

typedef void (*platform_event_callback_t)(platform_event_t* ev);

void Window_Init(int width, int height, const char* title);
void Window_Shutdown(void);
void Window_ProcessEvents(void);
void Window_Present(const void* pixels, int width, int height);
void Window_ShowError(const char* error);
void Window_InstallCrashHandler(void);
void Window_SetEventCallback(platform_event_callback_t callback);
