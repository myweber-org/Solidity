
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

void renameFileWithTimestamp(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "Error: File does not exist." << std::endl;
        return;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt{};
    localtime_r(&in_time_t, &bt);

    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y%m%d_%H%M%S_");
    std::string timestampPrefix = oss.str();

    fs::path parentDir = filePath.parent_path();
    fs::path newFileName = timestampPrefix + filePath.filename().string();
    fs::path newFilePath = parentDir / newFileName;

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << filePath.filename() << " -> " << newFileName << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    fs::path targetFile(argv[1]);
    renameFileWithTimestamp(targetFile);

    return 0;
}