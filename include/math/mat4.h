#pragma once
#include "vec3.h"
#include "vec4.h"

struct Mat4 {
    float m[16]; // 4x4 matrix stored in a 1D array

    Mat4(); // default constructor

    static Mat4 identity();
    static Mat4 translate(const Vec3& t);                                              // translation matrix
    static Mat4 scale(const Vec3& s);                                                  // scale matrix
    static Mat4 perspective(float fovY, float aspect, float nearPlane, float farPlane); // projection matrix
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);           // view matrix

    Mat4 operator*(const Mat4& other) const;  // matrix x matrix
    Vec4 operator*(const Vec4& v)     const;  // matrix x vector
};