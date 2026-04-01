#pragma once
#include <string>

class AppState
{
public:
    static constexpr const char* STATE_FILE = "app_state.txt";

    float meshColor[3] = {0.2f, 0.6f, 1.0f};
    bool useTexture = false;

    float rotX = 0.0f;
    float rotY = 0.0f;

    void Load();
    void Save() const;
};