#include "math/vec3.h"
#include <cmath>

Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    float mag = magnitude();
    if (mag > 0) {
        return { x / mag, y / mag, z / mag };
    }
    return { 0, 0, 0 };
}

float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return {
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

Vec3 Vec3::operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
Vec3 Vec3::operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
Vec3 Vec3::operator*(float s) const { return { x * s, y * s, z * s }; }

void Vec3::print() const {
    std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
}