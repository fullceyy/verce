set_project("Project")
    set_languages("c++23")
    set_toolchains("mingw")
    set_policy("generator.compile_commands", true)
    -- set_symbols("debug")
    -- add_requires("glfw", "glm", "assimp")
includes("ThirdParty")
includes("Sources")

-- set_project("Playground")
-- set_languages("c++23")
-- add_requires("assimp", {configs = {shared = false}})
-- set_plat("mingw")
-- set_targetdir("Exec")

-- local GLFW_DIR           = "ThirdParty/GLFW"
-- local GLAD_DIR           = "ThirdParty/glad"
-- local IMGUI_DIR          = "ThirdParty/imgui"
-- local IMGUI_BACKENDS_DIR = "ThirdParty/imgui/backends"
-- local GLM_DIR            = "ThirdParty/glm"
-- local STB_DIR            = "ThirdParty/stb"
-- local KHR_DIR            = "ThirdParty/glad/KHR"
-- local ASSIMP_DIR         = "ThirdParty/assimp"
-- local KTX_DIR            = "ThirdParty/ktx"
-- local STB_DIR            = "ThirdParty/stb"

-- target("Playground")
-- set_kind("binary")
-- set_rundir("$(projectdir)")

-- add_files("main.cpp")
-- add_files("Sources/**.cpp")
-- remove_files("Sources/ArchModel.cpp")

-- add_files(IMGUI_DIR .. "/*.cpp")
-- add_files(IMGUI_BACKENDS_DIR .. "/*.cpp")
-- add_files(GLM_DIR .. "/**.cpp")
-- add_files(GLAD_DIR .. "/src/glad.c")

-- add_includedirs("Sources")
-- add_includedirs("ThirdParty")
-- add_includedirs(GLFW_DIR)
-- add_includedirs(IMGUI_DIR)
-- add_includedirs(IMGUI_BACKENDS_DIR)
-- add_includedirs(GLM_DIR)
-- add_includedirs(STB_DIR)
-- add_includedirs(KHR_DIR)
-- add_includedirs(ASSIMP_DIR)
-- add_includedirs(KTX_DIR)

-- add_linkdirs(GLFW_DIR .. "/lib")
-- add_links("glfw3")

-- add_linkdirs(ASSIMP_DIR .. "/lib")
-- add_links("assimp")

-- add_linkdirs(KTX_DIR .. "/lib")
-- add_links("ktx")

-- add_syslinks("opengl32", "gdi32", "user32", "shell32", "z", "minizip")
-- --dogshit, add like an asset reader or something bruv.
-- after_build(function(target)
--     os.cp("assets",  target:targetdir())
-- end)

-- target_end()


-- --useful xmake commands--
--    xmake show
--    xmake -v