includes("Utils")
includes("Graphics")

add_rules("mode.debug", "mode.release")
-- xmake f -m release / debug
target("Sources")
    set_kind("binary")
    set_targetdir("$(projectdir)/Executable")
    add_deps("Utils")
    add_deps("Graphics")

    add_files("main.cpp")

    after_build(function(target)
        os.cp("assets",  target:targetdir())
    end)

target_end()