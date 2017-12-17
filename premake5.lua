workspace "no-dice"
  configurations { "debug", "release", "profile" }

project "no-dice"
  kind "WindowedApp"
  language "C++"
  cppdialect "C++17"
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

  deps = { "harfbuzz", "freetype2", "xcb", "xcb-keysyms", "epoxy" }

  for i, pkg in pairs (deps) do
    local cflags, ret = os.outputof ("pkg-config --cflags "..pkg)
    buildoptions { cflags }
    local libs, ret = os.outputof ("pkg-config --libs "..pkg)
    linkoptions { libs }
  end

  warnings "Extra"

  filter "configurations:debug"
    defines { "DEBUG" }
    symbols "On"
    optimize "Debug"

  filter "configurations:release"
    defines { "NDEBUG" }
    symbols "Off"
    optimize "Speed"

  filter "configurations:profile"
    defines { "NDEBUG", "PROFILING" }
    symbols "On"
    optimize "Speed"
    buildoptions { "-pg", "-no-pie" }
    linkoptions { "-pg", "-no-pie" }

  filter "toolset:gcc"
    buildoptions { "-flto", "-pthread" }
    linkoptions { "-flto", "-pthread" }

