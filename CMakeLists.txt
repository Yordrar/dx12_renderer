cmake_minimum_required( VERSION 3.18.3 )

project(dx12_renderer)

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

option(DX12RENDERER_BUILD_DEMO_APPS "Build the demo apps that come with the renderer" OFF)

add_subdirectory(dx12_renderer)
if(DX12RENDERER_BUILD_DEMO_APPS)
	add_subdirectory(demoApp)
	set_property(TARGET dx12_renderer PROPERTY VS_STARTUP_PROJECT demoApp)
endif()