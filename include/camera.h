#ifndef CAMERA_H
#define CAMERA_H
#define _USE_MATH_DEFINES

#include "math/vec3.h"
#include "math/mat4.h"
#include <vector>
#include <cmath>

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    Vec3 position;
    Vec3 front;
    Vec3 up;
    Vec3 right;
    Vec3 worldUp;

    float yaw;
    float pitch;

    float movementSpeed;
    float mouseSensitivity;
    float zoom; 

    Camera(Vec3 startPosition = Vec3(0.0f, 0.0f, 0.0f), Vec3 startUp = Vec3(0.0f, 1.0f, 0.0f), float startYaw = -90.0f, float startPitch = 0.0f)
        : front(Vec3(0.0f, 0.0f, -1.0f)), movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(45.0f) {
        position = startPosition;
        worldUp = startUp;
        yaw = startYaw;
        pitch = startPitch;
        updateCameraVectorsEuler();
    }

    Mat4 GetViewMatrix() {
        return Mat4::lookAt(position, position + front, up);
    }

    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        if (direction == FORWARD)
            position = position + front * velocity;
        if (direction == BACKWARD)
            position = position - front * velocity;
        if (direction == LEFT)
            position = position - right * velocity;
        if (direction == RIGHT)
            position = position + right * velocity;
        if (direction == UP)
            position = position + up * velocity;
        if (direction == DOWN)
            position = position - up * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        if (constrainPitch) {
            yaw += xoffset;
            pitch += yoffset;

            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
            updateCameraVectorsEuler();
        } else {
            front = rotateVector(front, right, yoffset).normalized();
            up = rotateVector(up, right, yoffset).normalized();
            
            front = rotateVector(front, up, -xoffset).normalized();
            
            right = front.cross(up).normalized();
            up = right.cross(front).normalized();
        }
    }

    void ProcessMouseScroll(float yoffset) {
        zoom -= yoffset;
        if (zoom < 1.0f) zoom = 1.0f;
        if (zoom > 45.0f) zoom = 45.0f;
    }

private:
    Vec3 rotateVector(Vec3 v, Vec3 axis, float angleDegrees) {
        float angle = angleDegrees * (float)(M_PI / 180.0);
        float c = cosf(angle);
        float s = sinf(angle);
        
        Vec3 crossProduct = axis.cross(v);
        float dotProduct = axis.x * v.x + axis.y * v.y + axis.z * v.z;
        
        Vec3 result;
        result.x = v.x * c + crossProduct.x * s + axis.x * dotProduct * (1.0f - c);
        result.y = v.y * c + crossProduct.y * s + axis.y * dotProduct * (1.0f - c);
        result.z = v.z * c + crossProduct.z * s + axis.z * dotProduct * (1.0f - c);
        return result;
    }

    void updateCameraVectorsEuler() {
        Vec3 newFront;
        newFront.x = cosf(yaw * (float)(M_PI / 180.0)) * cosf(pitch * (float)(M_PI / 180.0));
        newFront.y = sinf(pitch * (float)(M_PI / 180.0));
        newFront.z = sinf(yaw * (float)(M_PI / 180.0)) * cosf(pitch * (float)(M_PI / 180.0));
        front = newFront.normalized();
        
        right = front.cross(worldUp).normalized();
        up = right.cross(front).normalized();
    }
};

#endif