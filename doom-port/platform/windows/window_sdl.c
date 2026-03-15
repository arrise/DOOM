#include "../platform_window.h"
#include <SDL.h>
#include <stdio.h>

static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_texture = NULL;
static platform_event_callback_t g_event_callback = NULL;

static int MapKey(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_LEFT:   return 0xac; // KEY_LEFTARROW
        case SDLK_RIGHT:  return 0xae; // KEY_RIGHTARROW
        case SDLK_UP:     return 0xad; // KEY_UPARROW
        case SDLK_DOWN:   return 0xaf; // KEY_DOWNARROW
        case SDLK_ESCAPE: return 27;   // KEY_ESCAPE
        case SDLK_RETURN: return 13;   // KEY_ENTER
        case SDLK_TAB:    return 9;    // KEY_TAB
        case SDLK_F1:     return (0x80+0x3b);
        case SDLK_F2:     return (0x80+0x3c);
        case SDLK_F3:     return (0x80+0x3d);
        case SDLK_F4:     return (0x80+0x3e);
        case SDLK_F5:     return (0x80+0x3f);
        case SDLK_F6:     return (0x80+0x40);
        case SDLK_F7:     return (0x80+0x41);
        case SDLK_F8:     return (0x80+0x42);
        case SDLK_F9:     return (0x80+0x43);
        case SDLK_F10:    return (0x80+0x44);
        case SDLK_F11:    return (0x80+0x57);
        case SDLK_F12:    return (0x80+0x58);
        case SDLK_BACKSPACE: return 127; // KEY_BACKSPACE
        case SDLK_PAUSE:  return 0xff; // KEY_PAUSE
        case SDLK_RCTRL:
        case SDLK_LCTRL:  return (0x80+0x1d); // KEY_RCTRL
        case SDLK_RSHIFT:
        case SDLK_LSHIFT: return (0x80+0x36); // KEY_RSHIFT
        case SDLK_RALT:
        case SDLK_LALT:   return (0x80+0x38); // KEY_RALT
        case SDLK_EQUALS: return 0x3d; // KEY_EQUALS
        case SDLK_MINUS:  return 0x2d; // KEY_MINUS
        default:
            if (sym >= 'A' && sym <= 'Z')
                return (int)sym + ('a' - 'A'); // To lowercase
            if (sym >= 0 && sym <= 127)
                return (int)sym;
            return 0;
    }
}

void Window_SetEventCallback(platform_event_callback_t callback) {
    g_event_callback = callback;
}

void Window_Init(int width, int height, const char* title) {
    g_window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width * 3, height * 3, // Initial scale factor 3x
        SDL_WINDOW_RESIZABLE
    );

    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return;
    }

    // Ensure rendering scales automatically to fit window
    SDL_RenderSetLogicalSize(g_renderer, width, height);

    g_texture = SDL_CreateTexture(g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height);
}

void Window_Shutdown(void) {
    if (g_texture) {
        SDL_DestroyTexture(g_texture);
        g_texture = NULL;
    }
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Window_ProcessEvents(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            if (g_event_callback) {
                platform_event_t ev;
                ev.type = PLATFORM_EVENT_QUIT;
                g_event_callback(&ev);
            } else {
                exit(0);
            }
        }
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (g_event_callback && !event.key.repeat) {
                platform_event_t ev;
                ev.type = (event.type == SDL_KEYDOWN) ? PLATFORM_EVENT_KEYDOWN : PLATFORM_EVENT_KEYUP;
                ev.data1 = MapKey(event.key.keysym.sym);
                if (ev.data1 != 0) {
                    g_event_callback(&ev);
                }
            }
        }
        else if (event.type == SDL_MOUSEMOTION) {
            if (g_event_callback) {
                platform_event_t ev;
                ev.type = PLATFORM_EVENT_MOUSEMOVE;
                ev.data1 = 0; 
                ev.data2 = event.motion.xrel * 2; 
                ev.data3 = -event.motion.yrel * 2;
                if (ev.data2 != 0 || ev.data3 != 0) {
                    g_event_callback(&ev);
                }
            }
        }
    }
}

void Window_InstallCrashHandler(void) {
    // SDL handles some basic signals, but this is a no-op for now
}

void Window_ShowError(const char* error) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "DOOM Fatal Error", error, NULL);
}

void Window_Present(const void* pixels, int width, int height) {
    if (!g_texture || !g_renderer) return;

    SDL_UpdateTexture(g_texture, NULL, pixels, width * 4);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
}
