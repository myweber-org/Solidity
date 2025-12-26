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

int main() {
    std::string path;
    std::string baseName;
    
    std::cout << "Enter directory path: ";
    std::getline(std::cin, path);
    
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);
    
    if (!fs::exists(path) || !fs::is_directory(path)) {
        std::cerr << "Invalid directory path." << std::endl;
        return 1;
    }
    
    renameFilesSequentially(path, baseName);
    
    return 0;
}