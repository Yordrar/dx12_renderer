cmake_minimum_required( VERSION 3.18.3 )

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED True)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

file(GLOB_RECURSE dx12_renderer_src
    "src/*.h"
    "src/*.cpp"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${dx12_renderer_src})
add_library(dx12_renderer STATIC)
target_sources(dx12_renderer PRIVATE ${dx12_renderer_src})

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

target_include_directories(dx12_renderer PUBLIC
"${CMAKE_CURRENT_SOURCE_DIR}/src"
"${CMAKE_CURRENT_SOURCE_DIR}/external"
"${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/inc"
"${CMAKE_CURRENT_SOURCE_DIR}/external/WinPixEventRuntime/Include/WinPixEventRuntime"
)

set_property(TARGET dx12_renderer PROPERTY VS_CONFIGURATION_TYPE StaticLibrary)

target_link_libraries(dx12_renderer 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/lib/x64/dxcompiler.lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/WinPixEventRuntime/bin/x64/WinPixEventRuntime.lib
)