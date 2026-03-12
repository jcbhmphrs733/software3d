#pragma once // prevents multiple inclusions of this header file
#include <iostream>

struct Vec2 {
    float x, y; // public attributes for 2D vector components

    // Constructors
    Vec2(); // default constructor initializes to (0, 0)
    Vec2(float x, float y); // parameterized constructor

    //Method declarations
    float magnitude() const;
    void print() const;

    Vec2 operator+(const Vec2& other) const; // vector addition
    Vec2 operator-(const Vec2& other) const; // vector subtraction
    
};