add_rules("mode.debug", "mode.release")

set_languages("cxx17")

add_requires("hidapi")

target("AppleDisplayBrightnessController")
    set_kind("binary")
    add_packages("hidapi")
    add_files("src/**.cpp")
    add_extrafiles(".clang-format", "xmake.lua", "README.md")