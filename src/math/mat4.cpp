#include "math/mat4.h"

Mat4::Mat4() {
    // Initialize the matrix to the i
    for (int i = 0; i < 16; ++i) {
        m[i] = 0.0f; // create an empty matrix
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
    Mat4 result; // create a new Mat4 object to hold the result of the operation
    // matrix multiplication
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[col * 4 + row] = m[0 * 4 + row] * other.m[col * 4 + 0] +
                                      m[1 * 4 + row] * other.m[col * 4 + 1] +
                                      m[2 * 4 + row] * other.m[col * 4 + 2] +
                                      m[3 * 4 + row] * other.m[col * 4 + 3] ;
        }
    }
    return result;
}