#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "mesh/obj_loader.h"
#include "framebuffer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Software Rasterizer";
GLuint shaderProgram, VAO, VBO, EBO, textureID;

Framebuffer* fb = nullptr;

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

GLFWwindow* InitializeWindow() {
    if (!glfwInit()) return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    return window;
}

void SetupResources() {
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

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    fb = new Framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void RunRenderLoop(GLFWwindow* window) {
    uint32_t red = 0xFF0000FF;
    uint32_t green = 0x00FF00FF;
    int centerX = WINDOW_WIDTH / 2;
    int centerY = WINDOW_HEIGHT / 2;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        fb->clear(0x000000FF);


        for (int i = -10; i <= 10; ++i) {
            fb->setPixel(centerX + i, centerY, green);
            fb->setPixel(centerX, centerY + i, green);
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
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
    Mesh cube = loader.load("assets/models/cube.obj");

    std::cout << "Vertices loaded: " << cube.vertices.size() << "\n";
    for (const Vec3& v : cube.vertices) {
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
    std::vector<Vec2> screenVerts;
    for (const Vec3& v : cube.vertices) {
        // transform to clip space
        Vec4 clip = MVP * Vec4(v.x, v.y, v.z, 1.0f);

        // perspective divide -> NDC (normalized device coordinates, range [-1, 1])
        Vec3 ndc = Vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);

        // viewport transform -> pixel coordinates
        Vec2 pixel = Vec2(
            (ndc.x + 1.0f) / 2.0f * WINDOW_WIDTH,
            (1.0f - ndc.y) / 2.0f * WINDOW_HEIGHT  // y flipped: screen y goes downward
        );
        screenVerts.push_back(pixel);
    }

    // print screen positions to verify
    std::cout << "\nProjected screen positions:\n";
    for (size_t i = 0; i < screenVerts.size(); i++) {
        std::cout << "  v" << i << ": (" << screenVerts[i].x << ", " << screenVerts[i].y << ")\n";
    }
    // --- end transformation stage ---

    GLFWwindow* window = InitializeWindow(); 
    if (!window) {
        return -1;
    }
    
    SetupResources(); 
    RunRenderLoop(window);
    Cleanup(window);

    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}