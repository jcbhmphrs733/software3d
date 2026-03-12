#pragma once // prevents multiple inclusions of this header file
#include <iostream>

struct Vec4 {
    float x, y, z, w; // public attributes for 4D vector components

    // Constructors
    Vec4(); // default constructor initializes to (0, 0, 0, 0)
    Vec4(float x, float y, float z, float w); // parameterized constructor

    //Method declarations
    float magnitude() const;
    void print() const;

    Vec4 operator+(const Vec4& other) const; // vector addition
    Vec4 operator-(const Vec4& other) const; // vector subtraction
    
};