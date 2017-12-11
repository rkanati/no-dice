workspace "no-dice"
  configurations { "debug", "release", "profile" }

project "no-dice"
  kind "WindowedApp"
  language "C++"
  targetdir "%{cfg.buildcfg}/bin"

  files {
    "src/**.hpp",
    "src/**.cpp",
    "rk-core/src/**.cpp",
    "rk-rawio/src/**.cpp"
  }

  includedirs {
    "rk-core/include",
    "rk-math/include",
    "rk-rawio/include"
  }

  links { "xcb", "xcb-keysyms", "epoxy" }

  warnings "Extra"

  filter "configurations:debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:release"
    defines { "NDEBUG" }
    symbols "Off"
    optimize "On"
    buildoptions { "-flto", "-fuse-linker-plugin", "-fno-fat-lto-objects", "-O3" }
    linkoptions { "-flto", "-fuse-linker-plugin", "-fno-fat-lto-objects", "-O3" }

  filter "configurations:profile"
    defines { "NDEBUG", "PROFILING" }
    symbols "On"
    optimize "On"
    buildoptions { "-pg", "-no-pie" }
    linkoptions { "-pg", "-no-pie" }

  filter "toolset:gcc"
    buildoptions { "-std=c++17", "-pthread" }
    linkoptions { "-pthread" }

