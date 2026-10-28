-- include subprojects
includes("lib/commonlibf4")

-- name and version
local plugin_name = "HoldToActionAE"
local plugin_version = "1.0.0"
local plugin_version_major, plugin_version_minor, plugin_version_patch = plugin_version:match("^(%d+)%.(%d+)%.(%d+)$")

-- set project constants
set_project(plugin_name)
set_version(plugin_version)
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- add options
set_config("commonlib_toml", true)

-- override runtime count
add_defines("COMMONLIB_RUNTIMECOUNT=3")

-- windows header definitions
add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN")

-- define targets
-- define targets
target("detours")
    set_kind("static")
    set_languages("c++23")

    -- add src files
    add_files(
        "lib/detours/detours/Detours.cpp",
        "lib/detours/detours/Detours64.cpp",
        "lib/detours/detours/HideStaticLibSymbols.c"
    )
    add_headerfiles("lib/detours/detours/Detours.h")
    add_includedirs("lib/detours/detours", { public = true })
    add_includedirs(
        "lib/detours/detours/zydis/msvc",
        "lib/detours/detours/zydis/include",
        "lib/detours/detours/zydis/src",
        "lib/detours/detours/zydis/dependencies/zycore/include"
    )

	-- add defines
    add_defines("ZYDIS_STATIC_DEFINE")
target_end()

target(plugin_name)
    add_rules("commonlibf4.plugin", {
        name = plugin_name,
        author = "TheGamerX20",
        description = "Hold To Sneak & Sprint updated for NG & AE!"
    })

	-- add depds
	add_deps("detours")

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    -- pass name and version
    add_defines(
        'PLUGIN_NAME="' .. plugin_name .. '"',
        "PLUGIN_VERSION_MAJOR=" .. plugin_version_major,
        "PLUGIN_VERSION_MINOR=" .. plugin_version_minor,
        "PLUGIN_VERSION_PATCH=" .. plugin_version_patch
    )
