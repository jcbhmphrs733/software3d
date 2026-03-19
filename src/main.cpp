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
void processInput(GLFWwindow* window, float& rotX, float& rotY);

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Software Rasterizer";
GLuint shaderProgram, VAO, VBO, EBO, textureID;

Framebuffer* fb = nullptr;
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

void RunRenderLoop(GLFWwindow* window, const Mesh& mesh, const Mat4& view, const Mat4& projection) {
    // 1. Initialize random colors for each face of the mesh
    float rotX = 0.0f, rotY = 0.0f;
    std::srand((unsigned int)std::time(nullptr));
    const int faceCount = mesh.indices.size() / 3; 
    std::vector<uint32_t> faceColors(faceCount);
    for (int i = 0; i < faceCount; i++) {
        unsigned char r = (unsigned char)(std::rand() % 256);
        unsigned char g = (unsigned char)(std::rand() % 256);
        unsigned char b = (unsigned char)(std::rand() % 256);
        faceColors[i] = (r << 24) | (g << 16) | (b << 8) | 0xFF;
    }

    // 2. Pre-allocate storage for projected screen coordinates
    // This prevents thousands of allocations per second
    std::vector<Vec2> screenVerts(mesh.vertices.size());
    std::vector<float> screenDepths(mesh.vertices.size());

    while (!glfwWindowShouldClose(window)) {
        tracker.Tick();
        float time = (float)glfwGetTime();

        if (tracker.HasFpsUpdated()) {
            std::string title = "Software Rasterizer - FPS: " + std::to_string(tracker.GetFPS());
            glfwSetWindowTitle(window, title.c_str()); 
        }

       processInput(window, rotX, rotY);

        // 3. Clear the software buffers
        fb->clear(0x000000FF);
        fb->clearDepth();

        // 4. Dynamic Transformation Stage
        // SRT Order: Scale -> Rotate -> Translate (though translation is identity here)
        Mat4 model = rotateY(rotY) * rotateX(rotX) * scale(1.0f, 1.0f, 1.0f);
        Mat4 MVP = projection * view * model;

        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const Vec3& v = mesh.vertices[i];
            Vec4 clip = MVP * Vec4(v.x, v.y, v.z, 1.0f);
            
            // Perspective Divide: Transform from Clip Space to NDC
            Vec3 ndc = Vec3(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);

            // Viewport Transform: Map NDC [-1, 1] to Screen Pixels [0, Width/Height]
            screenVerts[i] = Vec2(
                (ndc.x + 1.0f) / 2.0f * WINDOW_WIDTH,
                (1.0f - ndc.y) / 2.0f * WINDOW_HEIGHT
            );
            screenDepths[i] = (ndc.z + 1.0f) / 2.0f; // Remap Depth to [0, 1]
        }

        // 5. Rasterization Stage (Triangles)
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            unsigned int i0 = mesh.indices[i];
            unsigned int i1 = mesh.indices[i+1];
            unsigned int i2 = mesh.indices[i+2];

            // Use the pre-calculated screen positions and depths
            DrawTriangle(*fb, 
                         screenVerts[i0], screenVerts[i1], screenVerts[i2], 
                         screenDepths[i0], screenDepths[i1], screenDepths[i2], 
                         faceColors[i / 3]);
        }

        // 6. Optional: Wireframe overlay
        /*
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            DrawLine(*fb, screenVerts[mesh.indices[i]], screenVerts[mesh.indices[i+1]], 0xFFFFFFFF);
            DrawLine(*fb, screenVerts[mesh.indices[i+1]], screenVerts[mesh.indices[i+2]], 0xFFFFFFFF);
            DrawLine(*fb, screenVerts[mesh.indices[i+2]], screenVerts[mesh.indices[i]], 0xFFFFFFFF);
        }
        */

        // 7. Upload the software framebuffer to the GPU texture
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, fb->getPixels());

        // 8. Render the full-screen quad using OpenGL
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
    Mesh cube = loader.load("assets/models/sphere.obj");

    Mat4 view = Mat4::lookAt(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
    Mat4 projection = Mat4::perspective(3.14159f / 4.0f, (float)800/600, 0.1f, 100.0f);

    GLFWwindow *window = InitializeWindow();
    if (!window) return -1;
    
    SetupResources();

    RunRenderLoop(window, cube, view, projection);

    Cleanup(window);
    return 0;
}


void processInput(GLFWwindow* window, float& rotX, float& rotY)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float speed = 0.5f;
    bool anyKey = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) { rotX -= speed; anyKey = true; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) { rotX += speed; anyKey = true; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) { rotY -= speed; anyKey = true; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { rotY += speed; anyKey = true; }

    static double lastInputTime = glfwGetTime();
    if (anyKey)
        lastInputTime = glfwGetTime();

    double idleSeconds = glfwGetTime() - lastInputTime;
    if (idleSeconds > 5.0) {
        rotY += speed * 0.5f;
        rotX += speed * 0.1f;
    }
}


void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

