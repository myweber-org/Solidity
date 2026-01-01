
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

void renameFileWithTimestamp(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "Error: File does not exist.\n";
        return;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file.\n";
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestamp;
    timestamp << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");

    fs::path parentPath = filePath.parent_path();
    fs::path newFileName = timestamp.str() + "_" + filePath.filename().string();
    fs::path newFilePath = parentPath / newFileName;

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << filePath.filename() << " -> " << newFileName << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    fs::path targetFile(argv[1]);
    renameFileWithTimestamp(targetFile);

    return 0;
}