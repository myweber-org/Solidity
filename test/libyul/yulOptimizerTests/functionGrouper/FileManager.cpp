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
    const std::string testContent = "This is sample content for testing.";

    if (SimpleFileManager::createFile(testFile, testContent)) {
        std::cout << "File created successfully.\n";
    } else {
        std::cout << "Failed to create file.\n";
        return 1;
    }

    if (SimpleFileManager::fileExists(testFile)) {
        std::cout << "File exists. Reading content...\n";
        std::string readContent = SimpleFileManager::readFile(testFile);
        std::cout << "Content: " << readContent << "\n";
    }

    if (SimpleFileManager::deleteFile(testFile)) {
        std::cout << "File deleted successfully.\n";
    } else {
        std::cout << "Failed to delete file.\n";
    }

    return 0;
}