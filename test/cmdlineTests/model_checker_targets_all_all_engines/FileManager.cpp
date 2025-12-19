
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

bool fileExists(const std::string& filePath) {
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

bool directoryExists(const std::string& dirPath) {
    return fs::exists(dirPath) && fs::is_directory(dirPath);
}

bool createDirectoryIfMissing(const std::string& dirPath) {
    if (directoryExists(dirPath)) {
        return true;
    }
    try {
        return fs::create_directories(dirPath);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }
}

std::string getFileExtension(const std::string& filePath) {
    fs::path pathObj(filePath);
    return pathObj.extension().string();
}

std::string getFileNameWithoutExtension(const std::string& filePath) {
    fs::path pathObj(filePath);
    return pathObj.stem().string();
}