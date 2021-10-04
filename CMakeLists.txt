cmake_minimum_required( VERSION 3.18.3 )

# Helper function to find all targets defined from a root dir
function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()

    get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)


### Main project
project(dx12_renderer)
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
"${CMAKE_SOURCE_DIR}/src"
"${CMAKE_SOURCE_DIR}/external/assimp/include"
)

set_property(TARGET dx12_renderer PROPERTY VS_STARTUP_PROJECT dx12_renderer)
set_property(TARGET dx12_renderer PROPERTY VS_CONFIGURATION_TYPE StaticLibrary)


## Add Demo App dir
add_subdirectory(demoApp)


### Link everything together
target_link_libraries( dx12_renderer 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC d3dcompiler.lib
)