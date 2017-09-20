workspace "luxa"
    configurations { "Debug", "Release" }
    location "build"
    architecture "x64"

project "application"
    kind "WindowedApp"
    language "C"
    targetdir "build/bin/%{cfg.buildcfg}"
    warnings "Extra"
    disablewarnings { "4204", "4100", "4152" }

    includedirs { "src" }

    libdirs "C://VulkanSDK//1.0.57.0//Lib"

    files { "src/application**.h", "src/application/**.c" }

    links { "luxa" }

    characterset "MBCS"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "luxa"
    kind "StaticLib"
    language "C"
    targetdir "build/bin/%{cfg.buildcfg}"
    warnings "Extra"
    disablewarnings { "4204", "4100", "4152" }

    includedirs { "src", "C://VulkanSDK//1.0.57.0//Include" }
    libdirs "C://VulkanSDK//1.0.57.0//Lib"

    files { "src/luxa/**.h", "src/luxa/**.c" }

    links { "vulkan-1" }

    characterset "MBCS"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

project "unit-tests"
    kind "ConsoleApp"
    language "C"
    targetdir "build/bin/%{cfg.buildcfg}"

    includedirs { "src", "." }

    files { "test/**.h", "test/**.c" }

    links { "luxa" }

    characterset "MBCS"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
