#pragma once // prevents multiple inclusions of this header file
#include <iostream>

struct Vec3 {
    float x, y, z; // public attributes for 3D vector components

    // Constructors
    Vec3(); // default constructor initializes to (0, 0, 0)
    Vec3(float x, float y, float z); // parameterized constructor

    //Method declarations
    float magnitude() const;
    void print() const;

    Vec3 operator+(const Vec3& other) const; // vector addition
    Vec3 operator-(const Vec3& other) const; // vector subtraction
    Vec3 operator*(float scalar)       const; // scalar multiplication

    float dot(const Vec3& other)   const; // dot product
    Vec3  cross(const Vec3& other) const; // cross product
    Vec3  normalized()             const; // unit vector in same direction
};