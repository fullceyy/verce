target("Graphics")
    set_kind("static")
    add_files("*.cpp")
    remove_files("AssimpModel.cpp")
    -- ThirdParty deps
    add_deps("glad", "GLFW", "glm", "ktx", "assimp", "stb", "imgui", "Utils")
    -- Source deps
    -- add_deps("Utils")
    -- add_syslinks("stdc++exp", {public = true})
target_end()