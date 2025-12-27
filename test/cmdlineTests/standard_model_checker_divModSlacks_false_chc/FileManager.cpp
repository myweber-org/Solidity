#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

class FileManager {
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

    static bool deleteFile(const std::string& filename) {
        return std::filesystem::remove(filename);
    }

    static bool fileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
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

    static bool writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    static size_t getFileSize(const std::string& filename) {
        if (!fileExists(filename)) {
            return 0;
        }
        return std::filesystem::file_size(filename);
    }
};

int main() {
    std::string testFile = "test_data.txt";
    std::string testContent = "This is a test file content.\nSecond line of content.";

    if (FileManager::createFile(testFile, testContent)) {
        std::cout << "File created successfully." << std::endl;
    }

    if (FileManager::fileExists(testFile)) {
        std::cout << "File exists. Size: " << FileManager::getFileSize(testFile) << " bytes." << std::endl;
        std::string content = FileManager::readFile(testFile);
        std::cout << "File content:\n" << content << std::endl;
    }

    if (FileManager::deleteFile(testFile)) {
        std::cout << "File deleted successfully." << std::endl;
    }

    return 0;
}