local vulkan_sdk_dir = "C://VulkanSDK//1.0.61.1"
local vulkan_include_dir = vulkan_sdk_dir .. "//Include"
local vulkan_lib_dir = vulkan_sdk_dir .. "//Lib"
local disabled_warnings = { "4204", "4100", "4152", "4201" }

workspace "luxa"
    configurations { "Debug", "Release" }
    location "build"
    architecture "x64"

project "application"
    kind "WindowedApp"
    language "C"
    targetdir "build/bin/%{cfg.buildcfg}"
    warnings "Extra"
    disablewarnings(disabled_warnings)

    includedirs { "src" }

    libdirs { vulkan_lib_dir }

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
    disablewarnings(disabled_warnings)

    includedirs { "src", vulkan_include_dir }
    libdirs { vulkan_lib_dir }

    files { "src/luxa/**.h", "src/luxa/**.c" }

    links { "vulkan-1" }

    characterset "MBCS"

    filter "configurations:Debug"
        defines { "DEBUG", "VK_USE_PLATFORM_WIN32_KHR" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "VK_USE_PLATFORM_WIN32_KHR" }
        optimize "On"
        symbols "On"

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
        symbols "On"
