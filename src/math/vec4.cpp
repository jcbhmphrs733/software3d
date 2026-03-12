#include "math/vec4.h"
#include <cmath>

// default constructor
Vec4::Vec4() {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 0.0f;
}

// parameterized constructor
Vec4::Vec4(float xval, float yval, float zval, float wval) {
    x = xval;
    y = yval;
    z = zval;
    w = wval;
}

// methods

float Vec4::magnitude() const {
    return std::sqrt(x * x + y * y + z * z + w * w); // calculate and return the magnitude of the vector
}

void Vec4::print() const {
    std::cout << "Vec4(" << x << ", " << y << ", " << z << ", " << w << ")\n"; // print the vector components
}


// operator overrides
Vec4 Vec4::operator+(const Vec4& other) const {
    return Vec4(x + other.x, y + other.y, z + other.z, w + other.w); // return the result of vector addition
}
Vec4 Vec4::operator-(const Vec4& other) const {
    return Vec4(x - other.x, y - other.y, z - other.z, w - other.w); // return the result of vector subtraction
}