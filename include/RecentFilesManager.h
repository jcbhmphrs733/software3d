#pragma once
#include <vector>
#include <string>

class RecentFilesManager {
private:
    std::vector<std::string> files;
    const int MAX_FILES = 5;
    const std::string CONFIG_FILE = "recent_files.txt";

public:
    RecentFilesManager();
    
    void Add(const std::string& filepath);
    void Load();
    void Save() const;
    
    const std::vector<std::string>& GetFiles() const { return files; }
    bool IsEmpty() const { return files.empty(); }
};