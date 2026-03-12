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
    return Vec3(x - other.x, y - other.y, z - other.z); // return the result of vector subtraction
}