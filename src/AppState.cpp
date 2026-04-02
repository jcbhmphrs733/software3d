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

    // save azimuth/elevation
    file << azimuth << " " << elevation << std::endl;

    // save ambient strength
    file << ambientStrength << std::endl;

    // save diffuse lighting
    file << diffuseLighting << std::endl;

    // save show outline
    file << showOutline << std::endl;

    // save depth falloff
    file << depthFalloff << std::endl;
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

    // load azimuth/elevation
    file >> azimuth >> elevation;

    // load ambient strength
    file >> ambientStrength;

    // load diffuse lighting
    file >> diffuseLighting;

    // load show outline
    file >> showOutline;

    // load depth falloff
    file >> depthFalloff;
}