
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
    std::tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);

    std::ostringstream timestampStream;
    timestampStream << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    std::string timestamp = timestampStream.str();

    fs::path parentDir = filePath.parent_path();
    std::string originalName = filePath.filename().string();
    fs::path newFilePath = parentDir / (timestamp + "_" + originalName);

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << originalName << " -> " << newFilePath.filename() << "\n";
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
    std::stringstream timestampStream;
    timestampStream << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");

    std::string timestamp = timestampStream.str();
    fs::path parentDir = filePath.parent_path();
    std::string originalName = filePath.filename().string();
    fs::path newPath = parentDir / (timestamp + "_" + originalName);

    try {
        fs::rename(filePath, newPath);
        std::cout << "Renamed: " << originalName << " -> " << newPath.filename() << std::endl;
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