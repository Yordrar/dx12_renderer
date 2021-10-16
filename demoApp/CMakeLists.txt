cmake_minimum_required( VERSION 3.18.3 )

### Demo App
project(demoApp)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED True)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

file(GLOB_RECURSE demoApp_src
    "src/*.h"
    "src/*.cpp"
)
file(GLOB_RECURSE demoApp_vertex_shaders
    "shader/*_vs.hlsl"
)
file(GLOB_RECURSE demoApp_pixel_shaders
    "shader/*_ps.hlsl"
)
file(GLOB_RECURSE demoApp_include_shaders
    "shader/*.hlsli"
)
set(demoApp_shader_src_all ${demoApp_vertex_shaders} ${demoApp_pixel_shaders} ${demoApp_include_shaders})
add_executable(demoApp WIN32 ${demoApp_src} ${demoApp_shader_src_all} )

source_group( "Shaders" FILES ${demoApp_shader_src_all} )
set_source_files_properties( ${demoApp_vertex_shaders} PROPERTIES
    VS_SHADER_DISABLE_OPTIMIZATIONS true
    VS_SHADER_ENABLE_DEBUG true
    VS_SHADER_ENTRYPOINT main_vs
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)
set_source_files_properties( ${demoApp_pixel_shaders} PROPERTIES
    VS_SHADER_DISABLE_OPTIMIZATIONS true
    VS_SHADER_ENABLE_DEBUG true
    VS_SHADER_ENTRYPOINT main_ps
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

target_include_directories(demoApp PUBLIC
"${CMAKE_SOURCE_DIR}/demoApp/src"
"${CMAKE_SOURCE_DIR}/demoApp/external/assimp/include"
)

set_property(TARGET demoApp PROPERTY VS_CONFIGURATION_TYPE Application)

target_link_libraries(demoApp
   PUBLIC dx12_renderer
   PUBLIC assimp
)


### External libraries
# assimp
add_subdirectory(external/assimp)
get_all_targets(assimp_targets external/assimp)
set_target_properties(${assimp_targets} PROPERTIES FOLDER assimp)
