cmake_minimum_required(VERSION 3.20)
project(FramePacerClone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wno-unused-parameter -O3 -m64)
add_link_options(-mwindows -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)

# Dear ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
)

add_executable(FramePacerClone WIN32
    src/main.cpp
    src/Pacer.cpp
    src/SystemManager.cpp
    src/GameDetector.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(FramePacerClone PRIVATE src ${IMGUI_DIR})

target_link_libraries(FramePacerClone PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    psapi
    winmm
)
