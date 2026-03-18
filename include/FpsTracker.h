#pragma once
#include <chrono>

class FpsTracker {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> previousTime;
    float fpsTimer = 0.0f;
    int frameCount = 0;
    
    float currentDeltaTime = 0.0f;
    int currentFPS = 0;
    bool fpsUpdatedThisFrame = false;

public:
    // Constructor: runs once when you create the tracker
    FpsTracker() {
        previousTime = std::chrono::high_resolution_clock::now();
    }

    void Tick() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime - previousTime;
        previousTime = currentTime;
        
        currentDeltaTime = elapsedTime.count();
        fpsTimer += currentDeltaTime;
        frameCount++;
        fpsUpdatedThisFrame = false; 

        // If 1 second has passed, calculate the new FPS
        if (fpsTimer >= 1.0f) {
            currentFPS = static_cast<int>(static_cast<float>(frameCount) / fpsTimer);
            fpsTimer = 0.0f;
            frameCount = 0;
            fpsUpdatedThisFrame = true; // Signal to main.cpp that a new FPS is ready
        }
    }

    // "Getters" so main.cpp can ask for the values safely
    float GetDeltaTime() const { return currentDeltaTime; }
    int GetFPS() const { return currentFPS; }
    bool HasFpsUpdated() const { return fpsUpdatedThisFrame; }
};