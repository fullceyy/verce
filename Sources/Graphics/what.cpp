// Sibenik Cathedral viewer: Assimp + OpenGL 3.3 core + GLFW + GLEW + GLM
// Usage: ./sibenik_viewer path/to/sibenik.obj
// Controls: WASD move, Q/E down/up, Shift = fast, mouse = look,
//           B = toggle bump mapping, F = wireframe, ESC = quit

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

// ---------------------------------------------------------------- shaders --
static const char* kVertSrc = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel, uView, uProj;

out vec3 vPos;      // world space
out vec3 vNormal;   // world space
out vec2 vUV;

void main() {
    vec4 w   = uModel * vec4(aPos, 1.0);
    vPos     = w.xyz;
    vNormal  = mat3(transpose(inverse(uModel))) * aNormal;
    vUV      = aUV;
    gl_Position = uProj * uView * w;
}
)GLSL";

static const char* kFragSrc = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uDiffuseMap;
uniform sampler2D uBumpMap;
uniform bool  uHasDiffuse;
uniform bool  uHasBump;
uniform bool  uUseBump;
uniform vec3  uKd;
uniform vec3  uKs;
uniform float uShininess;

uniform vec3  uViewPos;
uniform vec3  uLightPos;
uniform float uLightRange;
uniform float uBumpScale;

// Derivative-based bump mapping (Mikkelsen 2010): no tangents needed.
vec3 perturbNormal(vec3 N, vec3 p, float h) {
    vec3 dpdx = dFdx(p);
    vec3 dpdy = dFdy(p);
    float dhdx = dFdx(h);
    float dhdy = dFdy(h);
    vec3 r1 = cross(dpdy, N);
    vec3 r2 = cross(N, dpdx);
    float det = dot(dpdx, r1);
    vec3 grad = sign(det) * (dhdx * r1 + dhdy * r2);
    return normalize(abs(det) * N - uBumpScale * grad);
}

void main() {
    vec3 V = normalize(uViewPos - vPos);
    vec3 N = normalize(vNormal);
    if (dot(N, V) < 0.0) N = -N;               // two-sided shading

    if (uHasBump && uUseBump) {
        float h = texture(uBumpMap, vUV).r;
        N = perturbNormal(N, vPos, h);
    }

    vec3 albedo = uHasDiffuse ? texture(uDiffuseMap, vUV).rgb   // sRGB texture -> linear
                              : pow(uKd, vec3(2.2));

    vec3  L   = uLightPos - vPos;
    float d   = length(L);
    L /= d;
    float att = 1.0 / (1.0 + (d / uLightRange) * (d / uLightRange));

    float diff = max(dot(N, L), 0.0);
    vec3  H    = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), uShininess) * step(0.0, dot(N, L));

    vec3 ambient = 0.25 * albedo;
    vec3 color   = ambient + 2.0 * att * (diff * albedo + spec * uKs);

    FragColor = vec4(color, 1.0);               // GL_FRAMEBUFFER_SRGB does the gamma
}
)GLSL";

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n";
        std::exit(1);
    }
    return s;
}

static GLuint makeProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragSrc);
    GLuint p  = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::cerr << "Program link error:\n" << log << "\n";
        std::exit(1);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

// ------------------------------------------------------------------ model --
struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Material {
    GLuint    diffuseTex = 0;
    GLuint    bumpTex    = 0;
    glm::vec3 kd{0.8f};
    glm::vec3 ks{0.0f};
    float     shininess = 32.0f;
};

struct Mesh {
    GLuint   vao = 0, vbo = 0, ebo = 0;
    GLsizei  indexCount = 0;
    unsigned materialIndex = 0;
};

struct Model {
    std::vector<Mesh>     meshes;
    std::vector<Material> materials;
    glm::vec3 bmin{ 1e30f}, bmax{-1e30f};
};

static GLuint loadTexture(const fs::path& path, bool srgb) {
    static std::unordered_map<std::string, GLuint> cache;
    const std::string key = path.string() + (srgb ? "|srgb" : "|lin");
    if (auto it = cache.find(key); it != cache.end()) return it->second;

    int w = 0, h = 0, n = 0;
    // Assimp's aiProcess_FlipUVs flips V, so we do NOT flip on the stb side.
    unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &n, 4);
    if (!data) {
        std::cerr << "  [texture] FAILED to load " << path << " (" << stbi_failure_reason() << ")\n";
        cache[key] = 0;
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // UVs go beyond 0..1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (GLEW_EXT_texture_filter_anisotropic) {
        GLfloat maxAniso = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(16.0f, maxAniso));
    }

    std::cout << "  [texture] " << path.filename().string() << "  " << w << "x" << h << "\n";
    cache[key] = tex;
    return tex;
}

// MTL files often contain Windows-style paths or paths relative to odd places.
static fs::path resolveTexturePath(const fs::path& modelDir, const aiString& raw) {
    std::string p = raw.C_Str();
    std::replace(p.begin(), p.end(), '\\', '/');
    fs::path full = modelDir / p;
    if (fs::exists(full)) return full;
    return modelDir / fs::path(p).filename();          // fall back to just the file name
}

static bool getTexture(const aiMaterial* m, aiTextureType type, const fs::path& dir,
                       fs::path& out) {
    if (m->GetTextureCount(type) == 0) return false;
    aiString s;
    if (m->GetTexture(type, 0, &s) != AI_SUCCESS) return false;
    out = resolveTexturePath(dir, s);
    return true;
}

static Model loadModel(const std::string& objPath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        objPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |          // only used when the file has no normals
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs |
        aiProcess_ImproveCacheLocality |
        aiProcess_SortByPType);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << "\n";
        std::exit(1);
    }

    const fs::path dir = fs::path(objPath).parent_path();
    Model model;

    // ---- materials
    std::cout << "Materials (" << scene->mNumMaterials << "):\n";
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* am = scene->mMaterials[i];
        Material mat;

        aiString name;
        am->Get(AI_MATKEY_NAME, name);
        std::cout << " - " << name.C_Str() << "\n";

        aiColor3D kd(0.8f, 0.8f, 0.8f), ks(0.f, 0.f, 0.f);
        float shin = 32.0f;
        am->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
        am->Get(AI_MATKEY_COLOR_SPECULAR, ks);
        am->Get(AI_MATKEY_SHININESS, shin);
        mat.kd = {kd.r, kd.g, kd.b};
        mat.ks = {ks.r, ks.g, ks.b};
        mat.shininess = std::clamp(shin, 1.0f, 256.0f);

        fs::path p;
        if (getTexture(am, aiTextureType_DIFFUSE, dir, p))
            mat.diffuseTex = loadTexture(p, /*srgb=*/true);

        // OBJ "map_bump" / "bump" is imported by Assimp as aiTextureType_HEIGHT
        if (getTexture(am, aiTextureType_HEIGHT, dir, p) ||
            getTexture(am, aiTextureType_NORMALS, dir, p))
            mat.bumpTex = loadTexture(p, /*srgb=*/false);

        model.materials.push_back(mat);
    }

    // ---- meshes (OBJ has an identity node tree, so we skip node transforms)
    std::cout << "Meshes: " << scene->mNumMeshes << "\n";
    for (unsigned mi = 0; mi < scene->mNumMeshes; ++mi) {
        const aiMesh* am = scene->mMeshes[mi];
        if (!(am->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)) continue;

        std::vector<Vertex>   verts(am->mNumVertices);
        std::vector<unsigned> indices;
        indices.reserve(am->mNumFaces * 3);

        for (unsigned i = 0; i < am->mNumVertices; ++i) {
            Vertex& v = verts[i];
            v.pos = {am->mVertices[i].x, am->mVertices[i].y, am->mVertices[i].z};
            v.normal = am->HasNormals()
                     ? glm::vec3(am->mNormals[i].x, am->mNormals[i].y, am->mNormals[i].z)
                     : glm::vec3(0, 1, 0);
            v.uv = am->HasTextureCoords(0)
                 ? glm::vec2(am->mTextureCoords[0][i].x, am->mTextureCoords[0][i].y)
                 : glm::vec2(0);
            model.bmin = glm::min(model.bmin, v.pos);
            model.bmax = glm::max(model.bmax, v.pos);
        }
        for (unsigned f = 0; f < am->mNumFaces; ++f) {
            const aiFace& face = am->mFaces[f];
            if (face.mNumIndices != 3) continue;
            indices.insert(indices.end(), {face.mIndices[0], face.mIndices[1], face.mIndices[2]});
        }

        Mesh mesh;
        mesh.indexCount    = static_cast<GLsizei>(indices.size());
        mesh.materialIndex = am->mMaterialIndex;

        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);
        glGenBuffers(1, &mesh.ebo);
        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glBindVertexArray(0);

        model.meshes.push_back(mesh);
    }

    std::cout << "Bounds: min(" << model.bmin.x << ", " << model.bmin.y << ", " << model.bmin.z
              << ")  max(" << model.bmax.x << ", " << model.bmax.y << ", " << model.bmax.z << ")\n";
    return model;
}

// ----------------------------------------------------------------- camera --
struct Camera {
    glm::vec3 pos{0};
    float yaw = -90.0f, pitch = 0.0f;   // degrees; yaw -90 looks down -Z

    glm::vec3 front() const {
        float y = glm::radians(yaw), p = glm::radians(pitch);
        return glm::normalize(glm::vec3(cos(y) * cos(p), sin(p), sin(y) * cos(p)));
    }
    glm::mat4 view() const { return glm::lookAt(pos, pos + front(), glm::vec3(0, 1, 0)); }
};

static Camera g_cam;
static bool   g_firstMouse = true, g_bump = true, g_wire = false;
static double g_lastX = 0, g_lastY = 0;

static void cursorCb(GLFWwindow*, double x, double y) {
    if (g_firstMouse) { g_lastX = x; g_lastY = y; g_firstMouse = false; }
    const float sens = 0.1f;
    g_cam.yaw   += float(x - g_lastX) * sens;
    g_cam.pitch -= float(y - g_lastY) * sens;
    g_cam.pitch  = std::clamp(g_cam.pitch, -89.0f, 89.0f);
    g_lastX = x; g_lastY = y;
}

static void keyCb(GLFWwindow* w, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, GLFW_TRUE);
    if (key == GLFW_KEY_B) g_bump = !g_bump;
    if (key == GLFW_KEY_F) g_wire = !g_wire;
}

static void resizeCb(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

// ------------------------------------------------------------------- main --
int main(int argc, char** argv) {
    const std::string objPath = argc > 1 ? argv[1] : "sibenik/sibenik.obj";

    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Sibenik Cathedral", nullptr, nullptr);
    if (!window) { std::cerr << "Window creation failed\n"; glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, resizeCb);
    glfwSetCursorPosCallback(window, cursorCb);
    glfwSetKeyCallback(window, keyCb);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW init failed\n"; return 1; }
    glGetError();  // GLEW can leave a spurious GL_INVALID_ENUM behind

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    GLuint prog = makeProgram();
    Model  model = loadModel(objPath);

    // Frame the scene from its bounds so we don't depend on the model's units.
    const glm::vec3 center = 0.5f * (model.bmin + model.bmax);
    const glm::vec3 extent = model.bmax - model.bmin;
    const float     size   = glm::length(extent);
    g_cam.pos = center;
    const glm::vec3 lightPos = center + glm::vec3(0, 0.15f * extent.y, 0);

    const GLint uModel = glGetUniformLocation(prog, "uModel");
    const GLint uView  = glGetUniformLocation(prog, "uView");
    const GLint uProj  = glGetUniformLocation(prog, "uProj");

    glUseProgram(prog);
    glUniform1i(glGetUniformLocation(prog, "uDiffuseMap"), 0);
    glUniform1i(glGetUniformLocation(prog, "uBumpMap"), 1);
    glUniform3fv(glGetUniformLocation(prog, "uLightPos"), 1, glm::value_ptr(lightPos));
    glUniform1f(glGetUniformLocation(prog, "uLightRange"), size * 0.5f);
    glUniform1f(glGetUniformLocation(prog, "uBumpScale"), size * 0.0005f);  // tweak to taste
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float  dt  = float(now - last);
        last = now;

        // ---- input
        float speed = size * 0.15f * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 3.0f : 1.0f);
        glm::vec3 f = g_cam.front();
        glm::vec3 r = glm::normalize(glm::cross(f, glm::vec3(0, 1, 0)));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) g_cam.pos += f * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) g_cam.pos -= f * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) g_cam.pos += r * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) g_cam.pos -= r * speed * dt;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) g_cam.pos.y += speed * dt;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) g_cam.pos.y -= speed * dt;

        // ---- render
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        float aspect = fbh > 0 ? float(fbw) / float(fbh) : 1.0f;
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, size * 0.001f, size * 10.0f);

        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, g_wire ? GL_LINE : GL_FILL);

        glUseProgram(prog);
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(g_cam.view()));
        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(glGetUniformLocation(prog, "uViewPos"), 1, glm::value_ptr(g_cam.pos));
        glUniform1i(glGetUniformLocation(prog, "uUseBump"), g_bump);

        for (const Mesh& m : model.meshes) {
            const Material& mat = model.materials[m.materialIndex];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.diffuseTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mat.bumpTex);

            glUniform1i(glGetUniformLocation(prog, "uHasDiffuse"), mat.diffuseTex != 0);
            glUniform1i(glGetUniformLocation(prog, "uHasBump"), mat.bumpTex != 0);
            glUniform3fv(glGetUniformLocation(prog, "uKd"), 1, glm::value_ptr(mat.kd));
            glUniform3fv(glGetUniformLocation(prog, "uKs"), 1, glm::value_ptr(mat.ks));
            glUniform1f(glGetUniformLocation(prog, "uShininess"), mat.shininess);

            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}