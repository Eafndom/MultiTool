# FramePacer Clone

An independently implemented Windows application for high-resolution frame pacing, similar to FramePacer.

## Features
- Microsecond-precision frame timing engine.
- Synthetic console-based benchmarks (`--benchmark`).
- Process and game detection using PSAPI.
- Windows Tray support.
- Configurable global hotkeys.

## Build Instructions (Linux/MinGW)
1. Install MinGW and CMake: `sudo apt-get install mingw-w64 cmake g++`
2. Configure: `cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release -S . -B build`
3. Build: `cmake --build build --config Release`
