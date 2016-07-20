workspace "no-dice"
  configurations { "debug", "release", "profile" }

project "no-dice"
  kind "ConsoleApp"
  language "C++"
  targetdir "%{cfg.buildcfg}/bin"

  files { "src/**.hpp", "src/**.cpp" }

  includedirs { "rk-core/include", "rk-math/include" }
  links { "xcb", "X11-xcb", "X11", "GL", "GLU", "EGL", "xcb-keysyms" }

  warnings "Extra"

  filter "configurations:debug"
    defines { "DEBUG" }
    flags { "Symbols" }

  filter "configurations:release"
    defines { "NDEBUG" }
    optimize "On"

  filter "configurations:profile"
    defines { "NDEBUG" }
    flags { "Symbols" }
    optimize "On"
    buildoptions { "-pg" }
    linkoptions { "-pg" }

  filter "action:gmake" -- FIXME: this should be toolset:gcc, but toolset: is broken in premake5 as of 2015-09-01
    buildoptions { "-std=c++14" }

