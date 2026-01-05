
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesSequentially(const std::string& directoryPath, const std::string& baseName) {
    std::vector<fs::path> files;
    
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry.path());
        }
    }
    
    std::sort(files.begin(), files.end());
    
    int counter = 1;
    for (const auto& file : files) {
        std::string extension = file.extension().string();
        std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = file.parent_path() / newFileName;
        
        try {
            fs::rename(file, newPath);
            std::cout << "Renamed: " << file.filename() << " -> " << newFileName << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
        }
        
        counter++;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }
    
    std::string directoryPath = argv[1];
    std::string baseName = argv[2];
    
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        std::cerr << "Invalid directory path: " << directoryPath << std::endl;
        return 1;
    }
    
    renameFilesSequentially(directoryPath, baseName);
    
    return 0;
}