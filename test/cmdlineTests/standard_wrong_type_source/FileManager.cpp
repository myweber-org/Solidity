
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool writeToFile(const std::string& filePath, const std::string& content) {
        std::ofstream outFile(filePath);
        if (!outFile.is_open()) {
            std::cerr << "Error: Unable to open file " << filePath << " for writing." << std::endl;
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    static std::string readFromFile(const std::string& filePath) {
        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            std::cerr << "Error: Unable to open file " << filePath << " for reading." << std::endl;
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(inFile)),
                             std::istreambuf_iterator<char>());
        inFile.close();
        return content;
    }

    static bool createDirectory(const std::string& dirPath) {
        std::error_code ec;
        if (fs::create_directories(dirPath, ec)) {
            return true;
        } else {
            std::cerr << "Error: Failed to create directory " << dirPath
                      << " - " << ec.message() << std::endl;
            return false;
        }
    }

    static std::vector<std::string> listFiles(const std::string& dirPath) {
        std::vector<std::string> files;
        if (!fs::exists(dirPath)) {
            std::cerr << "Error: Directory " << dirPath << " does not exist." << std::endl;
            return files;
        }
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().filename().string());
            }
        }
        return files;
    }

    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath);
    }

    static bool deleteFile(const std::string& filePath) {
        std::error_code ec;
        if (fs::remove(filePath, ec)) {
            return true;
        } else {
            std::cerr << "Error: Failed to delete file " << filePath
                      << " - " << ec.message() << std::endl;
            return false;
        }
    }
};

int main() {
    FileManager::createDirectory("test_data");
    FileManager::writeToFile("test_data/sample.txt", "Hello, FileManager!");
    std::string content = FileManager::readFromFile("test_data/sample.txt");
    std::cout << "File content: " << content << std::endl;
    auto files = FileManager::listFiles("test_data");
    std::cout << "Files in directory: ";
    for (const auto& file : files) {
        std::cout << file << " ";
    }
    std::cout << std::endl;
    return 0;
}