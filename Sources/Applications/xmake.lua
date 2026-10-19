target("Applications")
    set_kind("static")
    -- add_files("*.cpp")
    -- ThirdParty deps
    add_deps("glad", "GLFW", "glm", "ktx", "assimp", "stb", "Utils", "Graphics")
    -- Source deps
    -- add_syslinks("stdc++exp", {public = true})
target_end()