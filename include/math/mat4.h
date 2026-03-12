#pragma once
#include "vec4.h"

struct Mat4 {
    float m[16]; // 4x4 matrix stored in a 1D array

    Mat4(); // default constructor

    static Mat4 identity(); // static method belonging to Mat4 struct only, not to any instance of Mat4
    Mat4 operator*(const Mat4& other) const; // matrix multiplication
};