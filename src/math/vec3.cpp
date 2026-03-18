#include "math/vec3.h"
#include <cmath>

// default constructor
Vec3::Vec3() {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

// parameterized constructor
Vec3::Vec3(float xval, float yval, float zval) {
    x = xval;
    y = yval;
    z = zval;
}

// methods

float Vec3::magnitude() const {
    return std::sqrt(x * x + y * y + z * z); // calculate and return the magnitude of the vector
}

void Vec3::print() const {
    std::cout << "Vec3(" << x << ", " << y << ", " << z << ")\n"; // print the vector components
}


// operator overrides
Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z); // return the result of vector addition
}
Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

Vec3 Vec3::normalized() const {
    float mag = magnitude();
    return Vec3(x / mag, y / mag, z / mag);
}