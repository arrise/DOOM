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

file(GLOB SOURCES "src/*.c")
file(GLOB PLATFORM_SOURCES "platform/windows/*.c")

add_executable(doom-port ${SOURCES} ${PLATFORM_SOURCES})
target_include_directories(doom-port PRIVATE src platform)
target_link_libraries(doom-port user32 gdi32 winmm)

