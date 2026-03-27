cmake_minimum_required(VERSION 3.16)
project(software3d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB MATH_SOURCES  src/math/*.cpp)
file(GLOB MESH_SOURCES  src/mesh/*.cpp)
file(GLOB IMGUI_SOURCES src/ui/*.cpp)

add_executable(software3d
    src/main.cpp
    src/glad.c
    src/framebuffer.cpp
    src/rasterizer.cpp
    src/RecentFilesManager.cpp
    src/texture.cpp
    ${MATH_SOURCES}
    ${MESH_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(software3d PRIVATE
    include
    include/ui
    include/nfd
    src
)

if(WIN32)
    target_sources(software3d PRIVATE src/nfd/nfd_win.cpp)
    target_link_libraries(software3d PRIVATE comdlg32 ole32)
elseif(APPLE)
    target_sources(software3d PRIVATE src/nfd/nfd_cocoa.m)
    target_compile_definitions(software3d PRIVATE NFD_MACOS_ALLOWEDCONTENTTYPES=0)
    target_link_libraries(software3d PRIVATE "-framework AppKit")
elseif(UNIX)
    target_sources(software3d PRIVATE src/nfd/nfd_gtk.cpp)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    target_include_directories(software3d PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(software3d PRIVATE ${GTK3_LIBRARIES})
endif()

target_link_libraries(software3d PRIVATE glfw OpenGL::GL)

add_custom_command(TARGET software3d POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_BINARY_DIR}/assets
)

