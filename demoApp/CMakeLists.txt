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

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED True)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

file(GLOB_RECURSE demoApp_src
    "src/*.h"
    "src/*.cpp"
)

add_executable(demoApp WIN32 ${demoApp_src} )

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

target_include_directories(demoApp PUBLIC
"${CMAKE_SOURCE_DIR}/demoApp/src"
"${CMAKE_SOURCE_DIR}/demoApp/external/assimp/include"
)

set_property(TARGET demoApp PROPERTY VS_CONFIGURATION_TYPE Application)
set_property(TARGET demoApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:demoApp>)

target_link_libraries(demoApp
   PUBLIC dx12_renderer
   PUBLIC assimp
)


### External libraries
# assimp
add_subdirectory(external/assimp)
get_all_targets(assimp_targets external/assimp)
set_target_properties(${assimp_targets} PROPERTIES FOLDER assimp)

# Copy dxcompiler and shaders to build folder
add_custom_command(TARGET demoApp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dx12_renderer/external/dxc/bin/x64/dxcompiler.dll $<TARGET_FILE_DIR:demoApp>)
add_custom_command(TARGET demoApp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dx12_renderer/external/dxc/bin/x64/dxil.dll $<TARGET_FILE_DIR:demoApp>)
add_custom_command(TARGET demoApp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dx12_renderer/shader $<TARGET_FILE_DIR:demoApp>/shader)
add_custom_command(TARGET demoApp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/demoApp/shader $<TARGET_FILE_DIR:demoApp>/shader)