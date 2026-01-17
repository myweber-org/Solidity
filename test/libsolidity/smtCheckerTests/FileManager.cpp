
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }

    static bool directoryExists(const std::string& dirPath) {
        return fs::exists(dirPath) && fs::is_directory(dirPath);
    }

    static bool createDirectory(const std::string& dirPath) {
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

    static uintmax_t getFileSize(const std::string& filePath) {
        if (!fileExists(filePath)) {
            return 0;
        }
        try {
            return fs::file_size(filePath);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error getting file size: " << e.what() << std::endl;
            return 0;
        }
    }
};

int main() {
    std::string testFile = "test.txt";
    std::string testDir = "test_directory";

    if (FileManager::createDirectory(testDir)) {
        std::cout << "Directory created or already exists: " << testDir << std::endl;
    }

    if (FileManager::fileExists(testFile)) {
        std::cout << "File exists: " << testFile << std::endl;
        std::cout << "File size: " << FileManager::getFileSize(testFile) << " bytes" << std::endl;
    } else {
        std::cout << "File does not exist: " << testFile << std::endl;
    }

    return 0;
}