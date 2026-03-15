cmake_minimum_required(VERSION 3.10)
project(doom-port C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

# We might need to define some macros or disable warnings
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
    # Doom uses a lot of POSIX functions which are deprecated in MSVC
    add_compile_definitions(_POSIX_)
endif()

add_compile_definitions(NORMALUNIX)

include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.2
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.8.0
)
set(SDL2MIXER_MIDI ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2_mixer)

file(GLOB SOURCES "src/*.c")
file(GLOB PLATFORM_SOURCES "platform/windows/*.c")

add_executable(doom-port ${SOURCES} ${PLATFORM_SOURCES})
target_include_directories(doom-port PRIVATE src platform)
target_link_libraries(doom-port user32 gdi32 winmm SDL2::SDL2-static SDL2_mixer::SDL2_mixer)

