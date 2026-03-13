# Software 3D Rasterizer
This project is a minimal software-based 3D rasterizer built from scratch in C++. The primary goal is to gain a deep, first-principles understanding of the graphics rendering pipeline by manually implementing the math and algorithms usually handled by a GPU.

# Project Overview
The rasterizer bypasses modern hardware acceleration to manually convert 3D geometric data into a 2D image. It uses OpenGL solely as a means to display the final pixel buffer to the screen.

# Core Features
3D Model Loading: Support for loading and parsing simple .obj files.

Transformation Pipeline: Implementation of Model, View, and Projection matrices to handle 3D space.

Coordinate Transformation: Converting 3D triangles into 2D screen space coordinates.

Triangle Rasterization: A custom algorithm to fill and render triangles onto a pixel grid.

Version Control: Managed via GitHub for iterative development and tracking.

# Technical Stack
Language: C++

Window Management: GLFW

Display API: OpenGL (for frame buffer presentation)

Build System: CMake / Standard C++ compiler


# Contributors
Trystan Jones - "Work Hard, Play Hard"

Jacob Humphreys - "If at first you do succeed, try to not look so suprised."

Erick Orellana - "It's never too late to learn something new."

Dakota Haithcock
