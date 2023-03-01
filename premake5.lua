workspace "dx12_renderer"
	configurations { "Debug", "Release" }
	system "Windows"
    architecture "x86_64"
	language "C++"
	cppdialect "C++20"
	location "build"
	startproject "demoApp"
	staticruntime "on"
	flags "MultiProcessorCompile"

filter "Debug"
	runtime "Debug"
	symbols "on"
	optimize "off"
	targetdir "build/bin/debug"
	debugdir "build/bin/debug"
	defines "_DEBUG"

filter "Release"
	runtime "Release"
	symbols "off"
	optimize "on"
	targetdir "build/bin/release"
	debugdir "build/bin/release"
	flags "LinkTimeOptimization"

project "dx12_renderer"
	kind "StaticLib"
	files { "dx12_renderer/src/**.h", "dx12_renderer/src/**.cpp" }
	libdirs { "dx12_renderer/external/dxc/lib/x64", "dx12_renderer/external/WinPixEventRuntime/bin/x64" }
	links { "d3d12", "dxgi", "dxguid", "dxcompiler", "WinPixEventRuntime" }
	defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
	includedirs { 
		"dx12_renderer/src",
		"dx12_renderer/external",
		"dx12_renderer/external/dxc/inc",
		"dx12_renderer/external/WinPixEventRuntime/Include/WinPixEventRuntime"
	}
	prebuildcommands {
		"{COPYFILE} " .. _WORKING_DIR .. "/dx12_renderer/external/dxc/bin/x64/dxcompiler.dll %{cfg.buildtarget.directory}",
		"{COPYFILE} " .. _WORKING_DIR .. "/dx12_renderer/external/dxc/bin/x64/dxil.dll %{cfg.buildtarget.directory}",
		"{COPYFILE} " .. _WORKING_DIR .. "/dx12_renderer/external/WinPixEventRuntime/bin/x64/WinPixEventRuntime.dll %{cfg.buildtarget.directory}",
		"{COPYDIR} " .. _WORKING_DIR .. "/dx12_renderer/shader %{cfg.buildtarget.directory}shader",
	}

project "demoApp"
	kind "WindowedApp"
	files { "demoApp/src/**.h", "demoApp/src/**.cpp" }
	libdirs "demoApp/external/"
	links { "dx12_renderer" }
	includedirs { 
		"demoApp/src", 
		"demoApp/external", 
		"dx12_renderer/src"
	}
	prebuildcommands {
		"{COPYDIR} " .. _WORKING_DIR .. "/demoApp/shader %{cfg.buildtarget.directory}shader",
		"{COPYDIR} " .. _WORKING_DIR .. "/demoApp/resource %{cfg.buildtarget.directory}resource",
	}