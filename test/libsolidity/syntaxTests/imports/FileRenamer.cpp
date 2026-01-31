
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
    std::string originalName = filePath.filename().string();
    fs::path newFilePath = parentDir / (timestamp + "_" + originalName);

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << originalName << " -> " << newFilePath.filename() << std::endl;
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
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath, const std::string& baseName) {
    try {
        fs::path dirPath(directoryPath);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        int counter = 1;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.path())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();

                std::ostringstream newFilename;
                newFilename << baseName << "_" << std::setw(4) << std::setfill('0') << counter << extension;
                fs::path newPath = oldPath.parent_path() / newFilename.str();

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
                    ++counter;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                }
            }
        }
        std::cout << "Renaming complete. Total files processed: " << (counter - 1) << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main() {
    std::string directory;
    std::string baseName;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);

    renameFilesInDirectory(directory, baseName);
    return 0;
}