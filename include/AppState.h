#pragma once
#include <string>

class AppState
{
public:
    static constexpr const char *STATE_FILE = "app_state.txt";

    float meshColor[3] = {0.2f, 0.6f, 1.0f};
    bool useTexture = false;
    std::string texturePath;    // path to UV texture, empty = use none

    float azimuth = 45.0f;        // degrees, horizontal rotation around Y axis
    float elevation = 45.0f;      // degrees above the horizon
    float ambientStrength = 0.2f; // minimum brightness [0,1]

    bool showOutline = false;

    std::string backgroundPath; // path to background image, empty = solid black
    std::string objPath;        // path to last loaded OBJ, empty = default

    void Load();
    void Save() const;
};