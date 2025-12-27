
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
        std::cerr << "Error: File does not exist.\n";
        return false;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file.\n";
        return false;
    }

    std::string timestamp = getCurrentTimestamp();
    fs::path newFilePath = filePath.parent_path() / (timestamp + "_" + filePath.filename().string());

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << filePath.filename() << " -> " << newFilePath.filename() << "\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << "\n";
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    fs::path targetFile(argv[1]);
    return renameFileWithTimestamp(targetFile) ? 0 : 1;
}