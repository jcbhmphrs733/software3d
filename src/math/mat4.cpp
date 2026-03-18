#include "math/mat4.h"
#include <cmath>

//default constructor
Mat4::Mat4() {
    // Initialize the matrix to all zeros
    for (int i = 0; i < 16; ++i) {
        m[i] = 0.0f; 
    }
}

Mat4 Mat4::identity() {
    Mat4 result; // create a new Mat4 object
    result.m[0] = 1.0f;
    result.m[5] = 1.0f;
    result.m[10] = 1.0f;
    result.m[15] = 1.0f;
    return result;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[col * 4 + row] = m[0 * 4 + row] * other.m[col * 4 + 0] +
                                      m[1 * 4 + row] * other.m[col * 4 + 1] +
                                      m[2 * 4 + row] * other.m[col * 4 + 2] +
                                      m[3 * 4 + row] * other.m[col * 4 + 3];
        }
    }
    return result;
}

Vec4 Mat4::operator*(const Vec4& v) const {
    return Vec4(
        m[0]*v.x + m[4]*v.y + m[8]*v.z  + m[12]*v.w,  // row 0
        m[1]*v.x + m[5]*v.y + m[9]*v.z  + m[13]*v.w,  // row 1
        m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]*v.w,  // row 2
        m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15]*v.w   // row 3
    );
}

Mat4 Mat4::translate(const Vec3& t) {
    Mat4 result = Mat4::identity();
    result.m[12] = t.x; // col 3, row 0
    result.m[13] = t.y; // col 3, row 1
    result.m[14] = t.z; // col 3, row 2
    return result;
}

Mat4 Mat4::scale(const Vec3& s) {
    Mat4 result = Mat4::identity();
    result.m[0]  = s.x; // col 0, row 0
    result.m[5]  = s.y; // col 1, row 1
    result.m[10] = s.z; // col 2, row 2
    return result;
}

Mat4 Mat4::perspective(float fovY, float aspect, float nearPlane, float farPlane) {
    Mat4 result; // zero matrix — perspective has no identity-like diagonal
    float f = 1.0f / std::tan(fovY / 2.0f);
    result.m[0]  = f / aspect;
    result.m[5]  = f;
    result.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    result.m[11] = -1.0f; // tells OpenGL this is a perspective (not ortho) projection
    result.m[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    return result;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 fwd   = (target - eye).normalized();       // direction camera looks
    Vec3 right = fwd.cross(up).normalized();        // camera's right axis
    Vec3 newUp = right.cross(fwd);                  // recomputed up (orthogonal)

    Mat4 result = Mat4::identity();
    // rotation part — each axis packed into a row (column-major storage)
    result.m[0] = right.x;  result.m[4] = right.y;  result.m[8]  = right.z;
    result.m[1] = newUp.x;  result.m[5] = newUp.y;  result.m[9]  = newUp.z;
    result.m[2] = -fwd.x;   result.m[6] = -fwd.y;   result.m[10] = -fwd.z;
    // translation part
    result.m[12] = -right.dot(eye);
    result.m[13] = -newUp.dot(eye);
    result.m[14] =  fwd.dot(eye);
    return result;
}