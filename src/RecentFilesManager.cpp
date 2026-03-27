#include "../include/RecentFilesManager.h"
#include <fstream>
#include <algorithm>

RecentFilesManager::RecentFilesManager()
{
    Load();
}

void RecentFilesManager::Add(const std::string &filepath)
{
    // Avoid Duplicates
    auto it = std::find(files.begin(), files.end(), filepath);
    if (it != files.end())
    {
        files.erase(it);
    }

    // Add to the front
    files.insert(files.begin(), filepath);

    // Limit 5
    if (files.size() > MAX_FILES)
    {
        files.pop_back();
    }
}

void RecentFilesManager::Load()
{
    std::ifstream file(CONFIG_FILE);
    std::string line;
    while (std::getline(file, line) && files.size() < MAX_FILES)
    {
        if (!line.empty())
        {
            files.push_back(line);
        }
    }
}

void RecentFilesManager::Save() const
{
    std::ofstream file(CONFIG_FILE);
    for (const auto &filepath : files)
    {
        file << filepath << "\n";
    }
}

void RecentFilesManager::Clear()
{
    files.clear();
}