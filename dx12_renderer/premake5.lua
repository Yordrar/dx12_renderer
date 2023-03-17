project "dx12_renderer"
	kind "StaticLib"
	files { "src/**.h", "src/**.cpp" }
	libdirs { "external/dxc/lib/x64", "external/WinPixEventRuntime/bin/x64" }
	links { "d3d12", "dxgi", "dxguid", "dxcompiler", "WinPixEventRuntime" }
	defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
	includedirs { 
		"src",
		"external",
		"external/dxc/inc",
		"external/WinPixEventRuntime/Include/WinPixEventRuntime"
	}
	prebuildcommands {
		"{COPYFILE} " .. _WORKING_DIR .. "/dx12_renderer/external/dxc/bin/x64/dxcompiler.dll %{cfg.buildtarget.directory}",
		"{COPYFILE} " .. _WORKING_DIR .. "/dx12_renderer/external/dxc/bin/x64/dxil.dll %{cfg.buildtarget.directory}",
		"{COPYFILE} " .. _WORKING_DIR .. "/dx12_renderer/external/WinPixEventRuntime/bin/x64/WinPixEventRuntime.dll %{cfg.buildtarget.directory}",
		"{COPYDIR} " .. _WORKING_DIR .. "/dx12_renderer/shader %{cfg.buildtarget.directory}shader",
	}
	pchheader "stdafx.h"
	pchsource "src/stdafx.cpp"
	forceincludes "stdafx.h"
