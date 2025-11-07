project "dx12_renderer"
	kind "StaticLib"
	pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"
	forceincludes "stdafx.h"
	files {
		"src/**.h",
		"src/**.cpp",
		"external/meshoptimizer/src/**",
		"external/tiny_obj_loader.h",
		"external/stb_image.h",
	}
	removefiles {
		"src/imgui/**",
	}
	files {
		"src/imgui/*.cpp",
		"src/imgui/backends/imgui_impl_win32.cpp",
		"src/imgui/backends/imgui_impl_dx12.cpp",
	}
	libdirs { "external/dxc/lib/x64", "external/WinPixEventRuntime/bin/x64" }
	links { "d3d12", "dxgi", "dxguid", "dxcompiler", "WinPixEventRuntime" }
	defines { "RENDERER", "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
	includedirs {
		"src",
		"src/imgui",
		"external/",
		"external/dx12_agility_sdk/build/native/include",
		"external/dxc/inc",
		"external/meshoptimizer/src",
		"external/WinPixEventRuntime/Include/WinPixEventRuntime",
		"external/imgui",
		"external/imgui/backends",
	}
