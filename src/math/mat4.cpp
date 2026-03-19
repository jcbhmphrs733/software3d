#include "math/mat4.h"
#include <cmath>
#include <cstring>

Mat4::Mat4() { std::memset(m, 0, sizeof(m)); }

Mat4 Mat4::identity() {
    Mat4 res;
    res.m[0] = 1.0f; res.m[5] = 1.0f; res.m[10] = 1.0f; res.m[15] = 1.0f;
    return res;
}

Mat4 Mat4::translate(const Vec3& t) {
    Mat4 res = identity();
    res.m[12] = t.x; res.m[13] = t.y; res.m[14] = t.z;
    return res;
}

Mat4 Mat4::scale(const Vec3& s) {
    Mat4 res = identity();
    res.m[0] = s.x; res.m[5] = s.y; res.m[10] = s.z;
    return res;
}

Mat4 Mat4::rotateX(float deg) {
    float r = deg * 3.14159265f / 180.0f;
    Mat4 res = identity();
    res.m[5] = cos(r);  res.m[6] = sin(r);
    res.m[9] = -sin(r); res.m[10] = cos(r);
    return res;
}

Mat4 Mat4::rotateY(float deg) {
    float r = deg * 3.14159265f / 180.0f;
    Mat4 res = identity();
    res.m[0] = cos(r);  res.m[2] = -sin(r);
    res.m[8] = sin(r);  res.m[10] = cos(r);
    return res;
}

Mat4 Mat4::rotateZ(float deg) {
    float r = deg * 3.14159265f / 180.0f;
    Mat4 res = identity();
    res.m[0] = cos(r);  res.m[1] = sin(r);
    res.m[4] = -sin(r); res.m[5] = cos(r);
    return res;
}

Mat4 Mat4::perspective(float fovY, float aspect, float n, float f) {
    Mat4 res;
    float tanHalfFov = tan(fovY / 2.0f);
    res.m[0] = 1.0f / (aspect * tanHalfFov);
    res.m[5] = 1.0f / (tanHalfFov);
    res.m[10] = -(f + n) / (f - n);
    res.m[11] = -1.0f;
    res.m[14] = -(2.0f * f * n) / (f - n);
    return res;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 z = (eye - target).normalized();
    Vec3 x = up.cross(z).normalized();
    Vec3 y = z.cross(x);

    Mat4 res = identity();
    res.m[0] = x.x; res.m[4] = x.y; res.m[8] = x.z;
    res.m[1] = y.x; res.m[5] = y.y; res.m[9] = y.z;
    res.m[2] = z.x; res.m[6] = z.y; res.m[10] = z.z;
    res.m[12] = -x.dot(eye); res.m[13] = -y.dot(eye); res.m[14] = -z.dot(eye);
    return res;
}

Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 res;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            for (int k = 0; k < 4; k++)
                res.m[c*4+r] += m[k*4+r] * o.m[c*4+k];
    return res;
}

Vec4 Mat4::operator*(const Vec4& v) const {
    return Vec4(
        m[0]*v.x + m[4]*v.y + m[8]*v.z + m[12]*v.w,
        m[1]*v.x + m[5]*v.y + m[9]*v.z + m[13]*v.w,
        m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]*v.w,
        m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15]*v.w
    );
}

Mat4 translate(float x, float y, float z) { return Mat4::translate({x,y,z}); }
Mat4 scale(float x, float y, float z) { return Mat4::scale({x,y,z}); }
Mat4 rotateX(float deg) { return Mat4::rotateX(deg); }
Mat4 rotateY(float deg) { return Mat4::rotateY(deg); }
Mat4 rotateZ(float deg) { return Mat4::rotateZ(deg); }