project "demoApp"
	kind "WindowedApp"
	files
	{
		"src/**.h",
		"src/**.cpp",
		"external/meshoptimizer/src/**",
	}
	libdirs "external/"
	links { "dx12_renderer" }
	defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
	includedirs { 
		"src", 
		"external", 
		"external/meshoptimizer/src",
		"../dx12_renderer/src",
		"../dx12_renderer/external/dx12_agility_sdk/build/native/include",
	}
	prebuildcommands {
		"{COPYDIR} " .. _WORKING_DIR .. "/demoApp/shader %{cfg.buildtarget.directory}shader",
		"{COPYDIR} " .. _WORKING_DIR .. "/demoApp/resource %{cfg.buildtarget.directory}resource",
	}
