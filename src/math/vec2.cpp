#include "math/vec2.h"
#include <cmath>

// default constructor
Vec2::Vec2() {
    x = 0.0f;
    y = 0.0f;
}

// parameterized constructor
Vec2::Vec2(float xval, float yval) {
    x = xval;
    y = yval;
}

// methods

float Vec2::magnitude() const {
    return std::sqrt(x * x + y * y); // calculate and return the magnitude of the vector
}

void Vec2::print() const {
    std::cout << "Vec2(" << x << ", " << y << ")\n"; // print the vector components
}


// operator overrides
Vec2 Vec2::operator+(const Vec2& other) const {
    return Vec2(x + other.x, y + other.y); // return the result of vector addition
}
Vec2 Vec2::operator-(const Vec2& other) const {
    return Vec2(x - other.x, y - other.y); // return the result of vector subtraction
}          