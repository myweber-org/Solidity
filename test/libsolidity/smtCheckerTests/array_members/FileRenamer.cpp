
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

bool renameFileWithTimestamp(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "Error: File does not exist." << std::endl;
        return false;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file." << std::endl;
        return false;
    }

    std::string timestamp = getCurrentTimestamp();
    fs::path parentDir = filePath.parent_path();
    std::string filename = filePath.filename().string();
    fs::path newFilePath = parentDir / (timestamp + "_" + filename);

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << filePath << " -> " << newFilePath << std::endl;
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

    fs::path targetFile(argv[1]);
    if (!renameFileWithTimestamp(targetFile)) {
        return 1;
    }

    return 0;
}