
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

bool renameFileWithTimestamp(const fs::path& filepath) {
    if (!fs::exists(filepath)) {
        std::cerr << "Error: File does not exist: " << filepath << std::endl;
        return false;
    }
    
    if (!fs::is_regular_file(filepath)) {
        std::cerr << "Error: Not a regular file: " << filepath << std::endl;
        return false;
    }
    
    std::string timestamp = getCurrentTimestamp();
    fs::path parentDir = filepath.parent_path();
    std::string filename = filepath.filename().string();
    fs::path newPath = parentDir / (timestamp + "_" + filename);
    
    try {
        fs::rename(filepath, newPath);
        std::cout << "Renamed: " << filepath << " -> " << newPath << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
        return 1;
    }
    
    fs::path filepath(argv[1]);
    return renameFileWithTimestamp(filepath) ? 0 : 1;
}