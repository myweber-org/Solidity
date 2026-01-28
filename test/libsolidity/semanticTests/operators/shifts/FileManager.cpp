#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool createFile(const std::string& path) {
        std::ofstream file(path);
        return file.is_open();
    }

    static bool deleteFile(const std::string& path) {
        return fs::remove(path);
    }

    static std::vector<std::string> listFiles(const std::string& directory) {
        std::vector<std::string> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            files.push_back(entry.path().filename().string());
        }
        return files;
    }

    static bool fileExists(const std::string& path) {
        return fs::exists(path);
    }
};

int main() {
    std::string testDir = "test_directory";
    fs::create_directory(testDir);

    std::string filePath = testDir + "/test_file.txt";
    
    if (FileManager::createFile(filePath)) {
        std::cout << "File created: " << filePath << std::endl;
    }

    auto files = FileManager::listFiles(testDir);
    std::cout << "Files in directory:" << std::endl;
    for (const auto& file : files) {
        std::cout << "  " << file << std::endl;
    }

    if (FileManager::fileExists(filePath)) {
        std::cout << "File exists: " << filePath << std::endl;
    }

    if (FileManager::deleteFile(filePath)) {
        std::cout << "File deleted: " << filePath << std::endl;
    }

    fs::remove(testDir);
    return 0;
}