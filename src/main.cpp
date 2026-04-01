#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "mesh/obj_loader.h"
#include "framebuffer.h"
#include "FpsTracker.h"
#include "rasterizer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "nfd.h"
#include "nfd_glfw3.h"
#include "RecentFilesManager.h"
#include "camera.h"

// Forward declarations
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void UpdateQuadVertices(int windowWidth);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow *window, double xposIn, double yposIn);

const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const int PANEL_WIDTH = 200;
const int WINDOW_WIDTH = VIEWPORT_WIDTH + PANEL_WIDTH;
const int WINDOW_HEIGHT = VIEWPORT_HEIGHT;
const char *WINDOW_TITLE = "Software Rasterizer";

int currentWindowWidth = WINDOW_WIDTH;
int currentWindowHeight = WINDOW_HEIGHT;

GLuint shaderProgram, VAO, VBO, EBO, textureID;
Framebuffer *fb = nullptr;
bool wireframeOnly = false;
FpsTracker tracker;

Camera camera(Vec3(0.0f, 0.0f, 3.0f));
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

// Mouse interaction state
float lastMouseX = 0.0f;
float lastMouseY = 0.0f;
bool  lmbDown    = false;   // left button held
bool  rmbDown    = false;   // right button held

// Scroll-based object depth offset (along camera forward axis)
float scrollOffset = 0.0f;

// Arcball / pan state — set by RunRenderLoop, read by callbacks
struct InteractionState {
    // Model matrix decomposed as: translate(pan) * rotate(rotation) * translate(scrollFwd)
    Mat4 rotation    = Mat4::identity();
    Vec3 pan         = Vec3(0.0f, 0.0f, 0.0f);   // world-space XY translation
    Vec3 pivotWorld  = Vec3(0.0f, 0.0f, 0.0f);   // arcball pivot in world space
    bool draggingRot = false;
    bool draggingPan = false;
} g_interaction;
size_t debugHitFace = SIZE_MAX; // index into indices[] of the last raycasted triangle

// Recent files manager global variable
RecentFilesManager recentFilesManager;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec2 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "out vec2 TexCoord;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
                                 "   TexCoord = aTexCoord;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "uniform sampler2D ourTexture;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = texture(ourTexture, TexCoord);\n"
                                   "}\n\0";

GLFWwindow *InitializeWindow()
{
    if (!glfwInit())
        return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    NFD_Init();

    return window;
}

void SetupResources()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float viewportRight = 1.0f - 2.0f * ((float)PANEL_WIDTH / (float)WINDOW_WIDTH);
    float vertices[] = {
        viewportRight, 1.0f, 1.0f, 1.0f,
        viewportRight, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f};
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    fb = new Framebuffer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
}

void UpdateQuadVertices(int windowWidth)
{
    float viewportRight = 1.0f - 2.0f * ((float)PANEL_WIDTH / (float)windowWidth);
    float vertices[] = {
        viewportRight, 1.0f, 1.0f, 1.0f,
        viewportRight, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f};
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}
void RunRenderLoop(GLFWwindow *window, const Mesh &mesh)
{
    Texture tex;
    bool useTexture = false;
    bool texLoaded = tex.load("assets/textures/metal.jpg");
    if (!texLoaded)
        std::cerr << "Warning: texture failed to load\n";
    std::vector<unsigned int> indices = mesh.indices;
    std::vector<Vec2> screenVerts(mesh.vertices.size());
    std::vector<float> screenDepths(mesh.vertices.size());
    std::vector<float> clipWs(mesh.vertices.size());

    Mesh currentMesh = mesh;

    // UI-controlled shading parameters
    float meshColor[3] = {0.2f, 0.6f, 1.0f}; // base RGB in [0,1]
    bool showOutline = false;

    // Lighting parameters
    float lightAzimuth = 45.0f;   // degrees, horizontal rotation around Y axis
    float lightElevation = 45.0f; // degrees above the horizon
    float ambientStrength = 0.2f; // minimum brightness [0,1]

    // Computes one unit face normal per triangle and stores in m.faceNormals.
    // For triangle (A,B,C): e1=B-A, e2=C-A, normal=normalize(cross(e1,e2)).
    auto computeFaceNormals = [](Mesh &m)
    {
        size_t faceCount = m.indices.size() / 3;
        m.faceNormals.resize(faceCount);
        for (size_t f = 0; f < faceCount; ++f)
        {
            Vec3 A = m.vertices[m.indices[f * 3]];
            Vec3 B = m.vertices[m.indices[f * 3 + 1]];
            Vec3 C = m.vertices[m.indices[f * 3 + 2]];
            Vec3 e1 = B - A;
            Vec3 e2 = C - A;
            m.faceNormals[f] = e1.cross(e2).normalized();
        }
    };

    // Average surrounding face normals into each vertex for Gouraud shading
    auto computeVertexNormals = [](Mesh &m)
    {
        m.vertexNormals.assign(m.vertices.size(), Vec3(0.0f, 0.0f, 0.0f));
        size_t faceCount = m.indices.size() / 3;
        for (size_t f = 0; f < faceCount; ++f)
        {
            const Vec3 &fn = m.faceNormals[f];
            m.vertexNormals[m.indices[f * 3    ]] = m.vertexNormals[m.indices[f * 3    ]] + fn;
            m.vertexNormals[m.indices[f * 3 + 1]] = m.vertexNormals[m.indices[f * 3 + 1]] + fn;
            m.vertexNormals[m.indices[f * 3 + 2]] = m.vertexNormals[m.indices[f * 3 + 2]] + fn;
        }
        for (auto &vn : m.vertexNormals)
            vn = vn.normalized();
    };

    computeFaceNormals(currentMesh);
    computeVertexNormals(currentMesh);

    std::string loadedFilePath;
    ObjLoader loader;

    while (!glfwWindowShouldClose(window))
    {
        tracker.Tick();

        if (tracker.HasFpsUpdated())
            glfwSetWindowTitle(window, WINDOW_TITLE);

        processInput(window);

        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        fb->clear(0x000000FF);
        fb->clearDepth();

        Mat4 view = camera.GetViewMatrix();
        Mat4 projection = Mat4::perspective(camera.zoom * 3.14159f / 180.0f, (float)VIEWPORT_WIDTH / (float)VIEWPORT_HEIGHT, 0.1f, 100.0f);
        Vec3 fwdOff = camera.front * (-scrollOffset);
        Mat4 model  = Mat4::translate(Vec3(g_interaction.pan.x + fwdOff.x,
                                           g_interaction.pan.y + fwdOff.y,
                                           g_interaction.pan.z + fwdOff.z))
                      * g_interaction.rotation;

        // On the first frame of LMB press, raycast through the cursor to set the arcball pivot.
        static bool prevLmbDown = false;
        if (lmbDown && !prevLmbDown)
        {
            float fovY_rad = camera.zoom * (3.14159265f / 180.0f);
            float tanHalf  = tanf(fovY_rad * 0.5f);
            float aspect   = (float)VIEWPORT_WIDTH / (float)VIEWPORT_HEIGHT;
            float ndcX = (lastMouseX / (float)VIEWPORT_WIDTH)  * 2.0f - 1.0f;
            // Y: match the mesh rendering convention ((1-ndc)*0.5*H → row, row displayed at H-row)
            // GLFW y=0 is visual top; the mesh maps ndc_y=+1 to framebuffer row 0 (visual bottom),
            // so the correct NDC y for a click is (y/H)*2-1, NOT 1-(y/H)*2.
            float ndcY = (lastMouseY / (float)VIEWPORT_HEIGHT) * 2.0f - 1.0f;
            // Build world-space ray from camera through the clicked pixel
            Vec3 rayDir = (camera.right * (ndcX * aspect * tanHalf)
                         + camera.up   * (ndcY * tanHalf)
                         + camera.front).normalized();
            Vec3 rayOrigin = camera.position;

            float closestT = 1e30f;
            Vec3  hitPoint;
            bool  hasHit = false;

            for (size_t ri = 0; ri < currentMesh.indices.size(); ri += 3)
            {
                unsigned int ii0 = currentMesh.indices[ri];
                unsigned int ii1 = currentMesh.indices[ri + 1];
                unsigned int ii2 = currentMesh.indices[ri + 2];
                Vec4 wA4 = model * Vec4(currentMesh.vertices[ii0].x, currentMesh.vertices[ii0].y, currentMesh.vertices[ii0].z, 1.0f);
                Vec4 wB4 = model * Vec4(currentMesh.vertices[ii1].x, currentMesh.vertices[ii1].y, currentMesh.vertices[ii1].z, 1.0f);
                Vec4 wC4 = model * Vec4(currentMesh.vertices[ii2].x, currentMesh.vertices[ii2].y, currentMesh.vertices[ii2].z, 1.0f);
                Vec3 wA(wA4.x, wA4.y, wA4.z);
                Vec3 wB(wB4.x, wB4.y, wB4.z);
                Vec3 wC(wC4.x, wC4.y, wC4.z);
                // Möller–Trumbore intersection (front-face only, matching the renderer's back-face cull)
                Vec3 e1 = wB - wA, e2 = wC - wA;
                Vec3 h   = rayDir.cross(e2);
                float det = e1.dot(h);
                if (det < 1e-6f) continue;  // skip back-facing and parallel triangles
                float invDet = 1.0f / det;
                Vec3  s = rayOrigin - wA;
                float u = s.dot(h) * invDet;
                if (u < 0.0f || u > 1.0f) continue;
                Vec3  q = s.cross(e1);
                float v = rayDir.dot(q) * invDet;
                if (v < 0.0f || u + v > 1.0f) continue;
                float rayT = e2.dot(q) * invDet;
                if (rayT > 0.0f && rayT < closestT)
                {
                    closestT = rayT;
                    hitPoint = rayOrigin + rayDir * rayT;
                    hasHit   = true;
                    debugHitFace = ri;
                }
            }
            if (hasHit)
                g_interaction.pivotWorld = hitPoint;
            else
            {
                // Place pivot on ray at the midpoint depth between nearest and farthest vertex
                float tMin = 1e30f, tMax = -1e30f;
                for (size_t vi = 0; vi < currentMesh.vertices.size(); ++vi)
                {
                    const Vec3& vert = currentMesh.vertices[vi];
                    Vec4 wV4 = model * Vec4(vert.x, vert.y, vert.z, 1.0f);
                    Vec3 wV(wV4.x, wV4.y, wV4.z);
                    float t = (wV - rayOrigin).dot(rayDir);
                    if (t < tMin) tMin = t;
                    if (t > tMax) tMax = t;
                }
                float tMid = (tMin + tMax) * 0.5f;
                if (tMid < 0.001f) tMid = 0.001f;
                g_interaction.pivotWorld = rayOrigin + rayDir * tMid;
            }
        }
        prevLmbDown = lmbDown;

        Mat4 MVP = projection * view * model;

        screenVerts.resize(currentMesh.vertices.size());
        screenDepths.resize(currentMesh.vertices.size());
        clipWs.resize(currentMesh.vertices.size());

        for (size_t i = 0; i < currentMesh.vertices.size(); ++i)
        {
            const Vec3 &v = currentMesh.vertices[i];
            Vec4 clip = MVP * Vec4(v.x, v.y, v.z, 1.0f);
            Vec3 ndc = Vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);
            screenVerts[i] = Vec2(
                (ndc.x + 1.0f) / 2.0f * VIEWPORT_WIDTH,
                (1.0f - ndc.y) / 2.0f * VIEWPORT_HEIGHT);
            screenDepths[i] = (ndc.z + 1.0f) / 2.0f;
            clipWs[i] = clip.w;
        }

        if (wireframeOnly)
        {
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                unsigned int i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
                DrawLine(*fb, screenVerts[i0], screenVerts[i1], screenDepths[i0], screenDepths[i1], 0xFFFFFFFF);
                DrawLine(*fb, screenVerts[i1], screenVerts[i2], screenDepths[i1], screenDepths[i2], 0xFFFFFFFF);
                DrawLine(*fb, screenVerts[i2], screenVerts[i0], screenDepths[i2], screenDepths[i0], 0xFFFFFFFF);
            }
        }
        else
        {
            uint32_t color = ((unsigned char)(meshColor[0] * 255) << 24) | ((unsigned char)(meshColor[1] * 255) << 16) | ((unsigned char)(meshColor[2] * 255) << 8) | 0xFF;

            // Build light direction from GUI azimuth/elevation angles
            float az = lightAzimuth * (3.14159265f / 180.0f);
            float el = lightElevation * (3.14159265f / 180.0f);
            Vec3 lightDir = Vec3(cosf(el) * sinf(az), sinf(el), cosf(el) * cosf(az)).normalized();

            // Pre-compute per-vertex light intensity for this frame (model matrix is constant)
            size_t vertCount = currentMesh.vertexNormals.size();
            std::vector<float> vertLightIntensity(vertCount, 0.0f);
            for (size_t v = 0; v < vertCount; ++v)
            {
                Vec4 tn = model * Vec4(currentMesh.vertexNormals[v].x,
                                       currentMesh.vertexNormals[v].y,
                                       currentMesh.vertexNormals[v].z, 0.0f);
                Vec3 worldNormal = Vec3(tn.x, tn.y, tn.z).normalized();
                float d = worldNormal.dot(lightDir);
                vertLightIntensity[v] = (d > 0.0f) ? d : 0.0f;
            }

            // Pass 1: fills
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                unsigned int i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
                Vec2 a = screenVerts[i0], b = screenVerts[i1], c = screenVerts[i2];
                float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
                if (area >= 0.0f)
                    continue;

                float lia = (i0 < vertCount) ? vertLightIntensity[i0] : 0.0f;
                float lib = (i1 < vertCount) ? vertLightIntensity[i1] : 0.0f;
                float lic = (i2 < vertCount) ? vertLightIntensity[i2] : 0.0f;

                Vec2 uva, uvb, uvc;
                if (useTexture && texLoaded && !currentMesh.uvs.empty() && i < currentMesh.uvIndices.size())
                {
                    uva = currentMesh.uvs[currentMesh.uvIndices[i]];
                    uvb = currentMesh.uvs[currentMesh.uvIndices[i + 1]];
                    uvc = currentMesh.uvs[currentMesh.uvIndices[i + 2]];
                }

                // Debug: highlight the raycasted hit face in yellow
                uint32_t faceColor = (i == debugHitFace) ? 0xFFFF00FF : color;

                DrawTriangle(*fb, a, b, c,
                             screenDepths[i0], screenDepths[i1], screenDepths[i2],
                             faceColor,
                             lia, lib, lic, ambientStrength,
                             (i == debugHitFace) ? nullptr : ((useTexture && texLoaded) ? &tex : nullptr),
                             uva, uvb, uvc,
                             clipWs[i0], clipWs[i1], clipWs[i2]);
            }

            if (showOutline)
            {
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                unsigned int i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
                Vec2 a = screenVerts[i0], b = screenVerts[i1], c = screenVerts[i2];
                float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
                if (area >= 0.0f)
                    continue;
                DrawLine(*fb, a, b, screenDepths[i0], screenDepths[i1], 0x000000FF);
                DrawLine(*fb, b, c, screenDepths[i1], screenDepths[i2], 0x000000FF);
                DrawLine(*fb, c, a, screenDepths[i2], screenDepths[i0], 0x000000FF);
            }
            }
        }

        // Draw trihedron (object local X/Y/Z axes) at the pivot point while LMB dragging
        if (g_interaction.draggingRot)
        {
            // pivotWorld is already in world space — project with VP (not MVP)
            Mat4 VP = projection * view;
            Vec4 pivotClip = VP * Vec4(g_interaction.pivotWorld.x,
                                       g_interaction.pivotWorld.y,
                                       g_interaction.pivotWorld.z, 1.0f);
            float origX, origY;
            if (pivotClip.w > 0.0f) {
                origX = ((pivotClip.x / pivotClip.w) + 1.0f) * 0.5f * VIEWPORT_WIDTH;
                // Use same Y convention as mesh rendering: (1-ndc)*0.5*H
                origY = (1.0f - (pivotClip.y / pivotClip.w)) * 0.5f * VIEWPORT_HEIGHT;
            } else {
                origX = 40.0f;
                origY = (float)(VIEWPORT_HEIGHT - 40);
            }
            const float axisLen = 25.0f;
            // Extract local axes from rotation matrix (column-major: col i = m[i*4..i*4+2])
            // Map to user convention: X=right, Y=forward(into screen), Z=up
            //   User X (red)   = column 0 (world X)
            //   User Y (green) = -column 2 (world -Z = forward away from camera)
            //   User Z (blue)  = column 1 (world Y = up)
            Vec3 axisX( g_interaction.rotation.m[0],  g_interaction.rotation.m[1],  g_interaction.rotation.m[2]);
            Vec3 axisY(-g_interaction.rotation.m[8], -g_interaction.rotation.m[9], -g_interaction.rotation.m[10]);
            Vec3 axisZ( g_interaction.rotation.m[4],  g_interaction.rotation.m[5],  g_interaction.rotation.m[6]);
            // Project each axis through view to a 2D screen offset
            Mat4 viewMat = camera.GetViewMatrix();
            auto projectAxis = [&](Vec3 axis) -> Vec2 {
                Vec4 v = viewMat * Vec4(axis.x, axis.y, axis.z, 0.0f);
                return Vec2(v.x * axisLen, -v.y * axisLen);
            };
            Vec2 origin2D(origX, origY);
            Vec2 px = projectAxis(axisX);
            Vec2 py = projectAxis(axisY);
            Vec2 pz = projectAxis(axisZ);
            DrawLine(*fb, origin2D, Vec2(origX + px.x, origY + px.y), 0.0f, 0.0f, 0xFF2020FF); // X red
            DrawLine(*fb, origin2D, Vec2(origX + py.x, origY + py.y), 0.0f, 0.0f, 0x20FF20FF); // Y green (forward)
            DrawLine(*fb, origin2D, Vec2(origX + pz.x, origY + pz.y), 0.0f, 0.0f, 0x2080FFFF); // Z blue (up)
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
                        GL_RGBA, GL_UNSIGNED_BYTE, fb->getPixels());

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2((float)(currentWindowWidth - PANEL_WIDTH), 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((float)PANEL_WIDTH, (float)currentWindowHeight), ImGuiCond_Always);
        ImGui::Begin("Scene", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Performance");
        ImGui::Separator();
        ImGui::Text("FPS:        %d", tracker.GetFPS());
        ImGui::Text("Frame time: %.2f ms", tracker.GetDeltaTime() * 1000.0f);

        ImGui::Spacing();
        ImGui::Text("Shading");
        ImGui::Separator();
        ImGui::Checkbox("Use Texture", &useTexture);
        ImGui::Checkbox("Show Outline", &showOutline);
        ImGui::ColorEdit3("Color", meshColor);
        ImGui::SliderFloat("Azimuth", &lightAzimuth, 0.0f, 360.0f);
        ImGui::SliderFloat("Elevation", &lightElevation, -90.0f, 90.0f);
        ImGui::SliderFloat("Ambient", &ambientStrength, 0.0f, 1.0f);

        ImGui::Spacing();
        ImGui::Text("OBJ Loader");
        ImGui::Separator();
        if (loadedFilePath.empty())
            ImGui::Text("No file loaded");
        else
            ImGui::TextWrapped("%s",
                               std::filesystem::path(loadedFilePath).filename().string().c_str());

        ImGui::Spacing();
        if (ImGui::Button("Open OBJ...", ImVec2(-1, 0)))
        {
            nfdu8char_t *outPath = nullptr;
            nfdfilteritem_t filters[] = {{"OBJ Files", "obj"}};
            nfdwindowhandle_t parentHandle = {};
            NFD_GetNativeWindowFromGLFWWindow(window, &parentHandle);
            nfdopendialogu8args_t args = {};
            args.filterList = filters;
            args.filterCount = 1;
            args.parentWindow = parentHandle;

            nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
            if (result == NFD_OKAY)
            {
                loadedFilePath = outPath;
                NFD_FreePathU8(outPath);

                recentFilesManager.Add(loadedFilePath);
                currentMesh = loader.load(loadedFilePath);
                computeFaceNormals(currentMesh);
                computeVertexNormals(currentMesh);
                indices = currentMesh.indices;
                screenVerts.resize(currentMesh.vertices.size());
                screenDepths.resize(currentMesh.vertices.size());
                g_interaction = InteractionState{};
                scrollOffset  = 0.0f;
            }
        }
        ImGui::Spacing();
        ImGui::Text("Recent Files");
        ImGui::Separator();

        if (recentFilesManager.IsEmpty())
        {
            ImGui::Text("No recent files.");
        }
        else
        {
            for (const auto &filepath : recentFilesManager.GetFiles())
            {
                if (ImGui::Button(std::filesystem::path(filepath).filename().string().c_str()))
                {
                    loadedFilePath = filepath;
                    currentMesh = loader.load(loadedFilePath);
                    computeFaceNormals(currentMesh);
                    computeVertexNormals(currentMesh);
                    indices = currentMesh.indices;
                    screenVerts.resize(currentMesh.vertices.size());
                    screenDepths.resize(currentMesh.vertices.size());
                    g_interaction = InteractionState{};
                    scrollOffset  = 0.0f;
                }
            }
        }

        if (ImGui::Button("Clear Recent Files", ImVec2(-1, 0)))
        {
            recentFilesManager.Clear();
            recentFilesManager.Save();
        }

        // Mesh Stats
        ImGui::Spacing();
        ImGui::Text("Mesh Stats:");
        ImGui::Separator();
        {
            size_t faceCount = currentMesh.indices.size() / 3;
            size_t vertCount = currentMesh.vertices.size();
            size_t edgeCount = faceCount * 3 / 2;

            ImGui::Text("Verts:    %zu", vertCount);
            ImGui::Text("Faces:    %zu", faceCount);
            ImGui::Text("Edges:    %zu", edgeCount);

            if (!loadedFilePath.empty())
            {
                std::error_code ec;
                auto bytes = std::filesystem::file_size(loadedFilePath, ec);
                if (!ec)
                {
                    if (bytes < 1024)
                        ImGui::Text("File size:     %zu B", bytes);
                    else if (bytes < 1024 * 1024)
                        ImGui::Text("File size:     %.1f KB", bytes / 1024.0);
                    else
                        ImGui::Text("File size:     %.1f MB", bytes / (1024.0 * 1024.0));
                }
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
void Cleanup(GLFWwindow *window)
{
    // Save Recent Files on Exit
    recentFilesManager.Save();

    NFD_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &textureID);

    delete fb;

    glfwTerminate();
}

int main()
{
    ObjLoader loader;
    Mesh mesh;
    std::string loadedFilePath;

    if (!recentFilesManager.IsEmpty())
    {
        loadedFilePath = recentFilesManager.GetFiles()[0];
        mesh = loader.load(loadedFilePath);
    }
    else
    {
        loadedFilePath = "assets/models/monkey.obj";
        mesh = loader.load(loadedFilePath);
    }

    GLFWwindow *window = InitializeWindow();
    if (!window)
        return -1;

    SetupResources();
    RunRenderLoop(window, mesh);
    Cleanup(window);

    recentFilesManager.Save();
    return 0;
}

void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    // Positive yoffset = scroll up = move object away; negative = closer
    scrollOffset -= (float)yoffset * 0.2f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    // Don't capture clicks that land on the ImGui panel
    if (ImGui::GetIO().WantCaptureMouse) return;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    lastMouseX = (float)mx;
    lastMouseY = (float)my;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        lmbDown = (action == GLFW_PRESS);
        g_interaction.draggingRot = lmbDown;
        if (action == GLFW_RELEASE)
            debugHitFace = SIZE_MAX;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rmbDown = (action == GLFW_PRESS);
        g_interaction.draggingPan = rmbDown;
    }
}

void cursor_pos_callback(GLFWwindow* /*window*/, double xposIn, double yposIn)
{
    if (ImGui::GetIO().WantCaptureMouse) return;

    float xpos = (float)xposIn;
    float ypos = (float)yposIn;
    float dx   = xpos - lastMouseX;
    float dy   = ypos - lastMouseY;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (g_interaction.draggingPan) {
        // Scale pan speed relative to viewport size
        const float panSpeed = 5.0f / VIEWPORT_WIDTH;
        Vec3 right = camera.right;
        Vec3 up    = camera.up;
        g_interaction.pan = g_interaction.pan + right * (dx * panSpeed)
                                               + up    * (dy * panSpeed);
    }

    if (g_interaction.draggingRot) {
        // Arcball: map mouse delta to rotation axis/angle
        float angleX = -dy * (3.14159265f / VIEWPORT_HEIGHT);  // pitch around camera right
        float angleY = dx * (3.14159265f / VIEWPORT_WIDTH);   // yaw around camera up

        Vec3 axisX = camera.right;
        Vec3 axisY = Vec3(0.0f, 1.0f, 0.0f);  // world Y keeps roll from drifting

        // Build incremental rotation: Ry * Rx
        Mat4 dRot = Mat4::identity();
        // Rodrigues rotation for axisY by angleY
        {
            float c = cosf(angleY), s = sinf(angleY);
            Vec3 a = axisY;
            Mat4 r = Mat4::identity();
            r.m[0]  = c + a.x*a.x*(1-c);     r.m[4]  = a.x*a.y*(1-c)-a.z*s; r.m[8]  = a.x*a.z*(1-c)+a.y*s;
            r.m[1]  = a.y*a.x*(1-c)+a.z*s;   r.m[5]  = c + a.y*a.y*(1-c);   r.m[9]  = a.y*a.z*(1-c)-a.x*s;
            r.m[2]  = a.z*a.x*(1-c)-a.y*s;   r.m[6]  = a.z*a.y*(1-c)+a.x*s; r.m[10] = c + a.z*a.z*(1-c);
            dRot = r;
        }
        {
            float c = cosf(angleX), s = sinf(angleX);
            Vec3 a = axisX;
            Mat4 r = Mat4::identity();
            r.m[0]  = c + a.x*a.x*(1-c);     r.m[4]  = a.x*a.y*(1-c)-a.z*s; r.m[8]  = a.x*a.z*(1-c)+a.y*s;
            r.m[1]  = a.y*a.x*(1-c)+a.z*s;   r.m[5]  = c + a.y*a.y*(1-c);   r.m[9]  = a.y*a.z*(1-c)-a.x*s;
            r.m[2]  = a.z*a.x*(1-c)-a.y*s;   r.m[6]  = a.z*a.y*(1-c)+a.x*s; r.m[10] = c + a.z*a.z*(1-c);
            dRot = dRot * r;
        }

        // Apply around pivot — must operate on the full model matrix, not just rotation,
        // otherwise the translation component shifts the pivot off the surface.
        Vec3 fwdOffRot = camera.front * (-scrollOffset);
        Mat4 fullModel  = Mat4::translate(Vec3(g_interaction.pan.x + fwdOffRot.x,
                                               g_interaction.pan.y + fwdOffRot.y,
                                               g_interaction.pan.z + fwdOffRot.z))
                          * g_interaction.rotation;
        Vec3 p = g_interaction.pivotWorld;
        Mat4 newModel = Mat4::translate(p) * dRot
                        * Mat4::translate(Vec3(-p.x, -p.y, -p.z)) * fullModel;
        // Re-decompose: upper 3x3 = new rotation, column 3 = new translation
        g_interaction.pan = Vec3(newModel.m[12] - fwdOffRot.x,
                                 newModel.m[13] - fwdOffRot.y,
                                 newModel.m[14] - fwdOffRot.z);
        Mat4 newR = Mat4::identity();
        newR.m[0] = newModel.m[0]; newR.m[1] = newModel.m[1]; newR.m[2] = newModel.m[2];
        newR.m[4] = newModel.m[4]; newR.m[5] = newModel.m[5]; newR.m[6] = newModel.m[6];
        newR.m[8] = newModel.m[8]; newR.m[9] = newModel.m[9]; newR.m[10] = newModel.m[10];
        g_interaction.rotation = newR;
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool fWasPressed = false;
    bool fIsPressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (fIsPressed && !fWasPressed)
        wireframeOnly = !wireframeOnly;
    fWasPressed = fIsPressed;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    currentWindowWidth = width;
    currentWindowHeight = height;
    glViewport(0, 0, width, height);
    UpdateQuadVertices(width);
}