
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class BasicFileManager {
public:
    static bool createFile(const std::string& filename, const std::string& content = "") {
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
    const std::string testContent = "Sample file content for testing.";

    if (BasicFileManager::createFile(testFile, testContent)) {
        std::cout << "File created successfully.\n";
    } else {
        std::cout << "Failed to create file.\n";
        return 1;
    }

    if (BasicFileManager::fileExists(testFile)) {
        std::cout << "File exists. Reading content:\n";
        std::string readContent = BasicFileManager::readFile(testFile);
        std::cout << readContent << "\n";
    }

    if (BasicFileManager::deleteFile(testFile)) {
        std::cout << "File deleted successfully.\n";
    } else {
        std::cout << "Failed to delete file.\n";
    }

    return 0;
}