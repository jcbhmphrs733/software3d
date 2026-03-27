#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <limits>
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

// Forward declarations
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, float &rotX, float &rotY);
void UpdateQuadVertices(int windowWidth); // must be declared before framebuffer_size_callback uses it

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
    // GL_DYNAMIC_DRAW — tells the GPU this buffer will be updated frequently (on every resize)
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

void RunRenderLoop(GLFWwindow *window, const Mesh &mesh, const Mat4 &view, const Mat4 &projection)
{
    float rotX = 0.0f, rotY = 0.0f;

    std::vector<unsigned int> indices = mesh.indices;
    std::vector<Vec2> screenVerts(mesh.vertices.size());
    std::vector<float> screenDepths(mesh.vertices.size());

    Mesh currentMesh = mesh;

    // UI-controlled shading parameters
    float meshColor[3] = {0.2f, 0.6f, 1.0f}; // base RGB in [0,1]
    float depthFalloff = 1.0f;               // 0 = flat, higher = darker at distance

    std::string loadedFilePath;
    ObjLoader loader;

    while (!glfwWindowShouldClose(window))
    {
        tracker.Tick();

        if (tracker.HasFpsUpdated())
            glfwSetWindowTitle(window, WINDOW_TITLE);

        processInput(window, rotX, rotY);

        fb->clear(0x000000FF);
        fb->clearDepth();

        Mat4 model = rotateY(rotY) * rotateX(rotX);
        Mat4 MVP = projection * view * model;

        screenVerts.resize(currentMesh.vertices.size());
        screenDepths.resize(currentMesh.vertices.size());

        for (size_t i = 0; i < currentMesh.vertices.size(); ++i)
        {
            const Vec3 &v = currentMesh.vertices[i];
            Vec4 clip = MVP * Vec4(v.x, v.y, v.z, 1.0f);
            Vec3 ndc = Vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);
            screenVerts[i] = Vec2(
                (ndc.x + 1.0f) / 2.0f * VIEWPORT_WIDTH,
                (1.0f - ndc.y) / 2.0f * VIEWPORT_HEIGHT);
            screenDepths[i] = (ndc.z + 1.0f) / 2.0f;
        }

        if (wireframeOnly)
        {
            // Wireframe mode: draw every edge with no culling so the full mesh is visible
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
            // Solid mode — two passes:
            // Pass 1: fill all front-facing triangles (builds the full depth buffer)
            // Pass 2: draw edges after, so they depth-test against every filled triangle

            float depthMin = std::numeric_limits<float>::max();
            float depthMax = std::numeric_limits<float>::lowest();
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                unsigned int i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
                Vec2 a = screenVerts[i0], b = screenVerts[i1], c = screenVerts[i2];
                float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
                if (area >= 0.0f)
                    continue;
                depthMin = std::min(depthMin, std::min({screenDepths[i0], screenDepths[i1], screenDepths[i2]}));
                depthMax = std::max(depthMax, std::max({screenDepths[i0], screenDepths[i1], screenDepths[i2]}));
            }

            uint32_t color = ((unsigned char)(meshColor[0] * 255) << 24) | ((unsigned char)(meshColor[1] * 255) << 16) | ((unsigned char)(meshColor[2] * 255) << 8) | 0xFF;

            // Pass 1: fills
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                unsigned int i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
                Vec2 a = screenVerts[i0], b = screenVerts[i1], c = screenVerts[i2];
                float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
                if (area >= 0.0f)
                    continue;
                DrawTriangle(*fb, a, b, c,
                             screenDepths[i0], screenDepths[i1], screenDepths[i2],
                             color, depthFalloff, depthMin, depthMax);
            }

            // Pass 2: edges — depth buffer is fully populated so edges correctly
            // occlude behind closer triangles; tiny bias beats same-triangle float noise
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

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
                        GL_RGBA, GL_UNSIGNED_BYTE, fb->getPixels());

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // ImGui panel — pinned to right edge, tracks currentWindowWidth/Height
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
        ImGui::ColorEdit3("Color", meshColor);
        ImGui::SliderFloat("Depth Falloff", &depthFalloff, 0.0f, 2.0f);

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
            nfdwindowhandle_t parentHandle;
            NFD_GetNativeWindowFromGLFWWindow(window, &parentHandle);
            nfdopendialogu8args_t args = {};
            args.filterList = filters;
            args.filterCount = 1;
            args.parentWindow = parentHandle;

            if (NFD_OpenDialogU8_With(&outPath, &args) == NFD_OKAY)
            {
                loadedFilePath = outPath;
                NFD_FreePathU8(outPath);

                recentFilesManager.Add(loadedFilePath);
                currentMesh = loader.load(loadedFilePath);
                indices = currentMesh.indices;
                screenVerts.resize(currentMesh.vertices.size());
                screenDepths.resize(currentMesh.vertices.size());
                rotX = rotY = 0.0f;
            }
        }

        // Adding recent files to the panel
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
                    indices = currentMesh.indices;
                    screenVerts.resize(currentMesh.vertices.size());
                    screenDepths.resize(currentMesh.vertices.size());
                    rotX = rotY = 0.0f;
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
            // For a closed manifold mesh edges = 3*faces/2; for open meshes this is an upper bound
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

    std::string loadedFilePath;

    Mesh mesh;

    if (!recentFilesManager.IsEmpty())
    {
        loadedFilePath = recentFilesManager.GetFiles()[0];
        mesh = loader.load(loadedFilePath);
    }
    else
    {
        mesh = loader.load("assets/models/monkey.obj"); // Default
    }
    // Mesh mesh = loader.load("assets/models/monkey.obj");

    Mat4 view = Mat4::lookAt(
        Vec3(0.0f, 0.0f, 5.0f),
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 1.0f, 0.0f));
    Mat4 projection = Mat4::perspective(
        3.14159f / 4.0f,
        (float)VIEWPORT_WIDTH / (float)VIEWPORT_HEIGHT,
        0.1f, 100.0f);

    GLFWwindow *window = InitializeWindow();
    if (!window)
        return -1;

    SetupResources();
    RunRenderLoop(window, mesh, view, projection);
    Cleanup(window);
    return 0;
}

void processInput(GLFWwindow *window, float &rotX, float &rotY)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float speed = 0.75f;
    bool anyKey = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        rotX -= speed;
        anyKey = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rotX += speed;
        anyKey = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        rotY -= speed;
        anyKey = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        rotY += speed;
        anyKey = true;
    }

    static double lastInputTime = glfwGetTime();
    if (anyKey)
        lastInputTime = glfwGetTime();

    if (glfwGetTime() - lastInputTime > 5.0)
    {
        rotY += speed * 0.5f;
        rotX += speed * 0.1f;
    }

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