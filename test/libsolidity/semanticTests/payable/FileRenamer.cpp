
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void batchRename(const std::string& directory, const std::string& baseName) {
    std::vector<fs::path> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry.path());
        }
    }
    
    std::sort(files.begin(), files.end());
    
    int counter = 1;
    for (const auto& file : files) {
        std::string extension = file.extension().string();
        std::string newName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = file.parent_path() / newName;
        
        try {
            fs::rename(file, newPath);
            std::cout << "Renamed: " << file.filename() << " -> " << newName << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Total files renamed: " << counter - 1 << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return 1;
    }
    
    batchRename(directory, baseName);
    return 0;
}