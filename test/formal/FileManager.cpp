
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath);
    }

    static long long getFileSize(const std::string& filePath) {
        if (!fileExists(filePath)) {
            return -1;
        }
        try {
            return fs::file_size(filePath);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            return -1;
        }
    }

    static void displayFileInfo(const std::string& filePath) {
        if (fileExists(filePath)) {
            std::cout << "File: " << filePath << std::endl;
            std::cout << "Size: " << getFileSize(filePath) << " bytes" << std::endl;
            std::cout << "Path: " << fs::absolute(filePath) << std::endl;
        } else {
            std::cout << "File does not exist: " << filePath << std::endl;
        }
    }
};

int main() {
    std::string testFile = "test.txt";
    FileManager::displayFileInfo(testFile);
    return 0;
}