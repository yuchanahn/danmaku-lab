cmake_minimum_required(VERSION 3.25)

project(DanmakuShooter
    VERSION 0.1.0
    LANGUAGES CXX
)

set(DANMAKU_CXX_STANDARD "23" CACHE STRING "C++ language standard (20 or 23)")
set_property(CACHE DANMAKU_CXX_STANDARD PROPERTY STRINGS 20 23)

if(NOT DANMAKU_CXX_STANDARD STREQUAL "20" AND NOT DANMAKU_CXX_STANDARD STREQUAL "23")
    message(FATAL_ERROR "DANMAKU_CXX_STANDARD must be 20 or 23")
endif()

file(GLOB DANMAKU_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(DanmakuShooter WIN32 ${DANMAKU_SOURCES})

add_executable(CollisionBenchmark
    tools/collision_benchmark.cpp
    src/Collision.cpp
    src/UniformGrid.cpp
)
target_include_directories(CollisionBenchmark PRIVATE src)
target_compile_features(CollisionBenchmark PRIVATE cxx_std_${DANMAKU_CXX_STANDARD})
set_target_properties(CollisionBenchmark PROPERTIES CXX_EXTENSIONS OFF)
if(MSVC)
    target_compile_options(CollisionBenchmark PRIVATE /W4 /permissive- /utf-8)
endif()

target_compile_features(DanmakuShooter PRIVATE cxx_std_${DANMAKU_CXX_STANDARD})

target_compile_definitions(DanmakuShooter PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(DanmakuShooter PRIVATE
    d3d11
    d3dcompiler
    dxgi
    d2d1
    dwrite
    windowscodecs
    xaudio2
)

# Copy even when only HLSL changes and the executable does not need relinking.
add_custom_target(CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:DanmakuShooter>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Sprite.hlsl"
        "$<TARGET_FILE_DIR:DanmakuShooter>/shaders/Sprite.hlsl"
)
add_dependencies(DanmakuShooter CopyShaders)

add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:DanmakuShooter>/assets"
)
add_dependencies(DanmakuShooter CopyAssets)

if(MSVC)
    target_compile_options(DanmakuShooter PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /utf-8
    )
else()
    target_compile_options(DanmakuShooter PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(DanmakuShooter PROPERTIES
    CXX_EXTENSIONS OFF
)
