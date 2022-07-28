cmake_minimum_required( VERSION 3.18.3 )

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED True)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

file(GLOB_RECURSE dx12_renderer_src
    "src/*.h"
    "src/*.cpp"
)
add_library(dx12_renderer STATIC ${dx12_renderer_src})

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

target_include_directories(dx12_renderer PUBLIC
"${CMAKE_SOURCE_DIR}/dx12_renderer/src"
)

set_property(TARGET dx12_renderer PROPERTY VS_CONFIGURATION_TYPE StaticLibrary)

target_link_libraries( dx12_renderer 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC ${CMAKE_SOURCE_DIR}/dx12_renderer/external/dxc/lib/x64/dxcompiler.lib
)