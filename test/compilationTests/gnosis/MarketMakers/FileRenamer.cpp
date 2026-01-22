
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

bool renameFileWithTimestamp(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "Error: File does not exist." << std::endl;
        return false;
    }

    fs::path pathObj(filePath);
    std::string timestamp = getCurrentTimestamp();
    std::string newFileName = timestamp + "_" + pathObj.filename().string();
    fs::path newPath = pathObj.parent_path() / newFileName;

    try {
        fs::rename(pathObj, newPath);
        std::cout << "Renamed '" << pathObj.filename().string()
                  << "' to '" << newFileName << "'" << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    if (!renameFileWithTimestamp(filePath)) {
        return 1;
    }

    return 0;
}