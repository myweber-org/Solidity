#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class SimpleFileManager {
public:
    static bool createFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

    static bool deleteFile(const std::string& filename) {
        return fs::remove(filename);
    }

    static bool fileExists(const std::string& filename) {
        return fs::exists(filename);
    }
};

int main() {
    const std::string testFile = "test_data.txt";
    const std::string testContent = "This is sample file content.\nLine 2 of content.";

    std::cout << "Creating file: " << testFile << std::endl;
    if (SimpleFileManager::createFile(testFile, testContent)) {
        std::cout << "File created successfully." << std::endl;
    } else {
        std::cout << "Failed to create file." << std::endl;
        return 1;
    }

    std::cout << "\nReading file content:" << std::endl;
    std::string readContent = SimpleFileManager::readFile(testFile);
    std::cout << readContent << std::endl;

    std::cout << "\nDeleting file: " << testFile << std::endl;
    if (SimpleFileManager::deleteFile(testFile)) {
        std::cout << "File deleted successfully." << std::endl;
    } else {
        std::cout << "Failed to delete file." << std::endl;
    }

    std::cout << "\nFile exists check: " 
              << (SimpleFileManager::fileExists(testFile) ? "Yes" : "No") 
              << std::endl;

    return 0;
}