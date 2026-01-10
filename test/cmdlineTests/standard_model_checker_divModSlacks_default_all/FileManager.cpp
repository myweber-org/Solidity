
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

bool fileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool directoryExists(const std::string& path) {
    return fs::exists(path) && fs::is_directory(path);
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

std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos && dotPos != filename.length() - 1) {
        return filename.substr(dotPos + 1);
    }
    return "";
}

void printFileInfo(const std::string& path) {
    if (!fileExists(path)) {
        std::cout << "File does not exist: " << path << std::endl;
        return;
    }
    
    try {
        fs::file_status status = fs::status(path);
        std::cout << "File: " << path << std::endl;
        std::cout << "Size: " << fs::file_size(path) << " bytes" << std::endl;
        std::cout << "Extension: " << getFileExtension(path) << std::endl;
        
        auto ftime = fs::last_write_time(path);
        std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
        std::cout << "Last modified: " << std::ctime(&cftime);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing file info: " << e.what() << std::endl;
    }
}