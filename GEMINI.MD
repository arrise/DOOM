# linuxdoom-1.10 → Windows 11 (x64) Port

## Project Goal

Port **linuxdoom-1.10** to **Windows 11 (64-bit)** while preserving the original gameplay and architecture.

The port should prioritize:

- Clean **modular design**
- **Platform abstraction**
- **Future extensibility**
- Ability to later improve rendering, networking, and gameplay systems

The end result should be a **clean, buildable Windows project** that runs the original DOOM engine using real DOOM WAD files.

This project should remain **non-commercial** and follow the original license constraints.

---

# Design Principles

## 1. Preserve the Original Engine Logic

Avoid rewriting gameplay or rendering logic during the port.

Focus on:

- platform compatibility
- build modernization
- modularization

Engine behavior must remain identical to the original Linux version unless explicitly stated.

---

## 2. Platform Abstraction

The original code mixes platform code with engine code.  
This must be separated.

Create a **platform layer** responsible for:

- window creation
- input
- audio
- timing
- filesystem access

Example structure:

src/
    engine/
        doom/
        render/
        game/
        wad/

platform/
    windows/
    linux/

framework/
    memory/
    logging/
    config/


The engine must **not call OS APIs directly**.

---

## 3. Future Extensibility

The architecture must support future improvements mentioned in the original README:

Possible future upgrades:

- improved renderer
- transparency
- look up/down
- slopes
- client/server networking
- hardware accelerated renderer
- modern timing systems

This means:

- avoid global dependencies
- isolate subsystems
- maintain clear interfaces

---

# Build System

Replace legacy build scripts with **CMake**.

Requirements:

- Visual Studio 2022 support
- Windows 11 x64 target
- Debug and Release configurations

Example command:

mkdir build
cd build
cmake ..
cmake --build .


---

# Directory Structure

doom-port/
CMakeLists.txt

src/
    engine/
        doom/
        render/
        game/
        wad/

    platform/
        windows/

    framework/
        memory/
        logging/
        config/

external/

assets/

docs/


---

# Phase Plan

## Phase 1 — Initial Compilation (Minimal Changes)

Goal: get the code **compiling on Windows** with minimal modification.

Tasks:

- import linuxdoom-1.10 source
- fix compiler errors
- remove Linux-only includes
- stub platform functions
- replace GCC-specific constructs if needed

Do NOT refactor yet.

Deliverable:

compiles successfully on Windows


---

## Phase 2 — Platform Layer

Goal: isolate OS functionality.

Create platform interfaces:

platform_window.h
platform_input.h
platform_audio.h
platform_timer.h
platform_filesystem.h


Then implement Windows versions:

platform/windows/window_win32.cpp
platform/windows/input_win32.cpp
platform/windows/audio_win32.cpp
platform/windows/timer_win32.cpp


Use:

- Win32 API for windowing
- DirectSound or modern equivalent
- high resolution timers

Deliverable:

engine no longer depends on Linux APIs


---

## Phase 3 — Rendering Output

Goal: display the original software renderer.

Options:

Option A (simplest):

- Win32 window
- framebuffer copied with GDI or Direct2D

Option B (recommended):

- SDL2 window
- upload framebuffer as texture

Keep renderer unchanged internally.

Deliverable:

DOOM renders correctly in a window


---

## Phase 4 — Input System

Replace Linux input handling with Windows input.

Support:

- keyboard
- mouse

Maintain original input behavior.

---

## Phase 5 — Audio

Replace Linux sound code.

Implement:

- sound effects
- music playback

Possible libraries:

- SDL2
- DirectSound
- OpenAL

Keep original sound mixing logic intact if possible.

---

## Phase 6 — Timing

DOOM uses a **35 Hz game tick**.

Implement a timing system that:

- preserves the 35Hz tick
- runs correctly on modern systems

Possible approach:

QueryPerformanceCounter


---

## Phase 7 — Code Cleanup

After successful port:

Refactor carefully:

- remove dead Linux code
- remove unused macros
- isolate global state where possible
- improve header organization

Do NOT change gameplay behavior.

---

# Optional Improvement Phases

These phases are **not required for the initial port** but should be supported by the architecture.

---

## Future Phase A — Renderer Improvements

Ideas from the original author:

- unified BSP traversal
- treat floors and ceilings as polygons
- render subsectors in a single pass

Possible improvements:

- OpenGL renderer
- Vulkan renderer
- hardware acceleration

---

## Future Phase B — Networking

Possible directions:

- packet server model
- client/server architecture

---

## Future Phase C — Gameplay Features

Examples:

- jumping
- crouching
- flying
- new weapons

---

# Constraints

Do NOT:

- change game logic
- rewrite renderer during port
- remove BSP system
- modify WAD format

---

# Testing

Use official DOOM WAD files.

Expected behaviors:

- levels load correctly
- monsters behave identically
- lighting and rendering match original DOOM
- game tick remains stable

---

# Final Deliverable

A Windows-native DOOM port with:

- CMake build system
- modular platform layer
- stable 35Hz game tick
- original gameplay preserved

Runs on:
Windows 11
x64
Visual Studio 2022


---

# Philosophy

The original code is old but valuable.  
Treat it as an **engine foundation**, not something to discard.

The goal is to:

port first
refactor second
improve third


Respect the design while making it maintainable for modern systems.