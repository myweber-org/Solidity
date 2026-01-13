
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
    std::tm tm_buf;
    localtime_r(&in_time_t, &tm_buf);

    std::ostringstream timestampStream;
    timestampStream << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    std::string timestamp = timestampStream.str();

    fs::path parentDir = filePath.parent_path();
    std::string originalName = filePath.filename().string();
    fs::path newFilePath = parentDir / (timestamp + "_" + originalName);

    if (fs::exists(newFilePath)) {
        std::cerr << "Error: Target file already exists." << std::endl;
        return;
    }

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << originalName << " -> " << newFilePath.filename() << std::endl;
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