#pragma once
#include "vec3.h"
#include "vec4.h"

struct Mat4 {
    float m[16]; 

    Mat4();
    static Mat4 identity();
    static Mat4 translate(const Vec3& t);
    static Mat4 scale(const Vec3& s);
    static Mat4 rotateX(float angleDeg);
    static Mat4 rotateY(float angleDeg);
    static Mat4 rotateZ(float angleDeg);
    
    static Mat4 perspective(float fovY, float aspect, float nearPlane, float farPlane);
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(const Vec4& v) const;
};


Mat4 translate(float x, float y, float z);
Mat4 scale(float x, float y, float z);
Mat4 rotateX(float deg);
Mat4 rotateY(float deg);
Mat4 rotateZ(float deg);