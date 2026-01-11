
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class SimpleFileManager {
public:
    static bool createFile(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << filePath << std::endl;
            return false;
        }
        file.close();
        std::cout << "File created: " << filePath << std::endl;
        return true;
    }

    static bool deleteFile(const std::string& filePath) {
        if (!fs::exists(filePath)) {
            std::cerr << "Error: File does not exist " << filePath << std::endl;
            return false;
        }
        if (!fs::is_regular_file(filePath)) {
            std::cerr << "Error: " << filePath << " is not a regular file" << std::endl;
            return false;
        }
        if (fs::remove(filePath)) {
            std::cout << "File deleted: " << filePath << std::endl;
            return true;
        } else {
            std::cerr << "Error: Could not delete file " << filePath << std::endl;
            return false;
        }
    }

    static std::vector<std::string> listFiles(const std::string& directoryPath) {
        std::vector<std::string> fileList;
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path " << directoryPath << std::endl;
            return fileList;
        }

        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                fileList.push_back(entry.path().filename().string());
            }
        }
        return fileList;
    }

    static void displayFileList(const std::vector<std::string>& files) {
        if (files.empty()) {
            std::cout << "Directory is empty or could not be read." << std::endl;
            return;
        }
        std::cout << "Files in directory:" << std::endl;
        for (const auto& file : files) {
            std::cout << "  - " << file << std::endl;
        }
    }
};

int main() {
    std::string testDir = "./test_directory";
    fs::create_directory(testDir);

    std::string file1 = testDir + "/example1.txt";
    std::string file2 = testDir + "/example2.txt";

    SimpleFileManager::createFile(file1);
    SimpleFileManager::createFile(file2);

    auto files = SimpleFileManager::listFiles(testDir);
    SimpleFileManager::displayFileList(files);

    SimpleFileManager::deleteFile(file1);

    files = SimpleFileManager::listFiles(testDir);
    SimpleFileManager::displayFileList(files);

    fs::remove_all(testDir);
    return 0;
}