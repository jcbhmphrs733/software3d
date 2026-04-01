#include "../include/AppState.h"
#include <fstream>

const char *STATE_FILE = "app_state.txt";

void AppState::Save() const
{
    std::ofstream file(STATE_FILE);
    if (!file.is_open())
        return;

    // save color
    file << meshColor[0] << " "
         << meshColor[1] << " "
         << meshColor[2] << std::endl;

    // save texture usage
    file << useTexture << std::endl;

    // save rotation
    file << rotX << " " << rotY << std::endl;
}

void AppState::Load()
{
    std::ifstream file(STATE_FILE);
    if (!file.is_open())
        return;

    // load color
    file >> meshColor[0] >> meshColor[1] >> meshColor[2];

    // load texture usage
    file >> useTexture;

    // load rotation
    file >> rotX >> rotY;
}