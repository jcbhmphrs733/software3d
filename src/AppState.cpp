#include "../include/AppState.h"
#include <fstream>
#include <sstream>

void AppState::Save() const
{
    // Write current UI/render settings so the next run restores the same scene setup.
    std::ofstream file(AppState::STATE_FILE);
    if (!file.is_open())
        return;

    file << "meshColor "       << meshColor[0] << " " << meshColor[1] << " " << meshColor[2] << "\n";
    file << "useTexture "      << useTexture      << "\n";
    file << "texturePath "     << texturePath     << "\n";
    file << "azimuth "         << azimuth         << "\n";
    file << "elevation "       << elevation       << "\n";
    file << "ambientStrength " << ambientStrength << "\n";
    file << "showOutline "     << showOutline     << "\n";
    file << "backgroundPath " << backgroundPath  << "\n";
    file << "objPath "         << objPath         << "\n";
}

void AppState::Load()
{
    // Restore previously saved UI/render settings if a state file exists.
    std::ifstream file(AppState::STATE_FILE);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line))
    {
        auto sep = line.find(' ');
        if (sep == std::string::npos)
            continue;
        std::string key = line.substr(0, sep);
        std::string val = line.substr(sep + 1);

        if (key == "meshColor") {
            std::istringstream ss(val);
            ss >> meshColor[0] >> meshColor[1] >> meshColor[2];
        } else if (key == "useTexture") {
            useTexture = (val == "1");
        } else if (key == "texturePath") {
            texturePath = val;
        } else if (key == "azimuth") {
            azimuth = std::stof(val);
        } else if (key == "elevation") {
            elevation = std::stof(val);
        } else if (key == "ambientStrength") {
            ambientStrength = std::stof(val);
        } else if (key == "showOutline") {
            showOutline = (val == "1");
        } else if (key == "backgroundPath") {
            backgroundPath = val;
        } else if (key == "objPath") {
            objPath = val;
        }
    }
}