#pragma once
#include <string>

class AppState
{
public:
    static constexpr const char *STATE_FILE = "app_state.txt";

    float meshColor[3] = {0.2f, 0.6f, 1.0f};
    bool useTexture = false;

    float rotX = 0.0f;
    float rotY = 0.0f;

    float azimuth = 45.0f;        // degrees, horizontal rotation around Y axis
    float elevation = 45.0f;      // degrees above the horizon
    float ambientStrength = 0.2f; // minimum brightness [0,1]

    bool diffuseLighting = true;

    bool showOutline = false;

    float depthFalloff = 1.0f; // 0 = flat, higher = darker at distance

    void Load();
    void Save() const;
};