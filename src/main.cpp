#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "mesh/obj_loader.h"
#include "framebuffer.h"
#include "FpsTracker.h"
#include "rasterizer.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Software Rasterizer";
GLuint shaderProgram, VAO, VBO, EBO, textureID;

Framebuffer* fb = nullptr;
bool wireframeOnly = false;
FpsTracker tracker;

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


// Ask GLFW to create an OS window and hook OpenGL into it. Returning a pointer to the window, or nullptr on failure.
GLFWwindow *InitializeWindow()
{
    if (!glfwInit())
        return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    //registering a function that GLFW uses to change the viewport when the window is resized.
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    return window;
}

void SetupResources() {

    // GLuint are unsigned integers that act as an ID for the shader living on the GPU. We can use these IDs to reference the shader when we want to use it.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // After compiling the shaders, we need to link them into a shader program that can be used for rendering. The shader program is also identified by an GLuint ID.
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Once the shaders are linked into a program, we can delete the individual shader objects as they are no longer needed.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // a full-screen quad (two triangles) with interleaved position and UV attributes
    float vertices[] = {
         1.0f,  1.0f,   1.0f, 1.0f, 
         1.0f, -1.0f,   1.0f, 0.0f, 
        -1.0f, -1.0f,   0.0f, 0.0f, 
        -1.0f,  1.0f,   0.0f, 1.0f  
    };
    unsigned int indices[] = {
        0, 1, 3, 
        1, 2, 3  
    };

    // VAO (Vertex Array Object) stores the configuration of vertex attributes and which VBOs to use. 
    glGenVertexArrays(1, &VAO);

    // VBO (Vertex Buffer Object) is a buffer on the GPU that holds vertex data.
    glGenBuffers(1, &VBO);

    // EBO (Element Buffer Object) is a buffer that holds indices for indexed drawing, allowing us to reuse vertices.
    glGenBuffers(1, &EBO);

    // activates the VAO
    glBindVertexArray(VAO);

    // binds the VBO and uploads the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // binds the EBO and uploads the index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // configure vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // The second attribute is the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // create an OpenGL texture to hold the framebuffer's pixel data
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // set texture parameters for nearest-neighbor sampling (no interpolation)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // allocate texture storage without initializing it
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // create the framebuffer that will hold our rendered pixel data before we upload it to the GPU texture
    fb = new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
}



void RunRenderLoop(GLFWwindow* window,
                   const std::vector<Vec2>& screenVerts,
                   const std::vector<float>& screenDepths,
                   const std::vector<unsigned int>& indices) {

    
    std::srand((unsigned int)std::time(nullptr));
    size_t triangleCount = indices.size() / 3;
    std::vector<uint32_t> faceColors(triangleCount);
    for (size_t i = 0; i < triangleCount; i++) {
        unsigned char r = (unsigned char)(std::rand() % 256);
        unsigned char g = (unsigned char)(std::rand() % 256);
        unsigned char b = (unsigned char)(std::rand() % 256);
        faceColors[i] = (r << 24) | (g << 16) | (b << 8) | 0xFF;
    }

    while (!glfwWindowShouldClose(window)) {

        tracker.Tick();
        float deltaTime = tracker.GetDeltaTime();

        if (tracker.HasFpsUpdated()) {
            std::string title = "Software Rasterizer - FPS: " + std::to_string(tracker.GetFPS());
            glfwSetWindowTitle(window, title.c_str()); 
        }

        processInput(window);

        // clear the framebuffer's color and depth buffers at the start of each frame
        fb->clear(0x000000FF);
        // every frame resets each pixel's depth to the far plane, so we can correctly render new geometry on top
        fb->clearDepth();

        // rasterize every triangle with depth testing
        if (!wireframeOnly) {
            for (size_t i = 0; i < indices.size(); i += 3) {
                unsigned int i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];
                Vec2 a = screenVerts[i0], b = screenVerts[i1], c = screenVerts[i2];
                float za = screenDepths[i0], zb = screenDepths[i1], zc = screenDepths[i2];
                uint32_t color = faceColors[i / 3];
                DrawTriangle(*fb, a, b, c, za, zb, zc, color);
            }
        }

        // draw wireframe edges on top (skip back-facing triangles)
        for (size_t i = 0; i < indices.size(); i += 3) {
            Vec2 a = screenVerts[indices[i]];
            Vec2 b = screenVerts[indices[i+1]];
            Vec2 c = screenVerts[indices[i+2]];
            // signed area: positive = front-facing, negative = back-facing
            float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (area <= 0.0f) continue;
            DrawLine(*fb, a, b, 0xFFFFFFFF);
            DrawLine(*fb, b, c, 0xFFFFFFFF);
            DrawLine(*fb, c, a, 0xFFFFFFFF);
        }

        // upload the framebuffer's pixel data to the GPU texture so it can be drawn on the full-screen quad
        glBindTexture(GL_TEXTURE_2D, textureID);
        // glTexSubImage2D updates a portion of the texture with new pixel data. Here we update the entire texture with the contents of our framebuffer.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, fb->getPixels());

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Cleanup(GLFWwindow* window) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &textureID);
    
    delete fb;
    
    glfwTerminate();
}

int main() {
    ObjLoader loader;
    Mesh cube = loader.load("assets/models/monkey.obj");

    std::cout << "Vertices loaded: " << cube.vertices.size() << "\n";
    for (const Vec3 &v : cube.vertices)
    {
        std::cout << "  v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    std::cout << "Indices loaded: " << cube.indices.size() << "\n";
    std::cout << "Triangles: " << cube.indices.size() / 3 << "\n";

    // --- Transformation stage ---
    // 1. build the three matrices
    Mat4 model      = Mat4::identity();
    Mat4 view       = Mat4::lookAt(
                        Vec3(0.0f, 0.0f, 3.0f),   // eye: camera sits 3 units back on Z
                        Vec3(0.0f, 0.0f, 0.0f),   // target: looking at the origin
                        Vec3(0.0f, 1.0f, 0.0f)    // up: world up is +Y
                      );
    Mat4 projection = Mat4::perspective(
                        3.14159f / 4.0f,                          // fovY: 45 degrees in radians
                        (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, // aspect ratio
                        0.1f,                                      // near plane
                        100.0f                                     // far plane
                      );
    Mat4 MVP = projection * view * model;

    // 2. project each vertex to screen space
    std::vector<Vec2>  screenVerts;
    std::vector<float> screenDepths; // NDC z per vertex, range [0,1]
    for (const Vec3& v : cube.vertices) {
        Vec4 clip = MVP * Vec4(v.x, v.y, v.z, 1.0f);
        Vec3 ndc  = Vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);

        Vec2 pixel = Vec2(
            (ndc.x + 1.0f) / 2.0f * WINDOW_WIDTH,
            (1.0f - ndc.y) / 2.0f * WINDOW_HEIGHT
        );
        screenVerts.push_back(pixel);
        screenDepths.push_back((ndc.z + 1.0f) / 2.0f); // remap [-1,1] -> [0,1]
    }

    // print screen positions to verify
    std::cout << "\nProjected screen positions:\n";
    for (size_t i = 0; i < screenVerts.size(); i++) {
        std::cout << "  v" << i << ": (" << screenVerts[i].x << ", " << screenVerts[i].y << ")\n";
    }
    // --- end transformation stage ---

    GLFWwindow *window = InitializeWindow();
    if (!window)
    {
        return -1;
    }
    
    SetupResources();
    RunRenderLoop(window, screenVerts, screenDepths, cube.indices);
    Cleanup(window);

    return 0;
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
    glViewport(0, 0, width, height);
}