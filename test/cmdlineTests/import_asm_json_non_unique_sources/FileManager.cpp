
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
        std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Unable to open file " << filePath << " for writing." << std::endl;
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    static std::string readFromFile(const std::string& filePath) {
        std::ifstream inFile(filePath, std::ios::in);
        if (!inFile.is_open()) {
            std::cerr << "Error: Unable to open file " << filePath << " for reading." << std::endl;
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(inFile)),
                             std::istreambuf_iterator<char>());
        inFile.close();
        return content;
    }

    static bool copyFile(const std::string& sourcePath, const std::string& destPath) {
        std::error_code ec;
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "Error copying file: " << ec.message() << std::endl;
            return false;
        }
        return true;
    }

    static bool deleteFile(const std::string& filePath) {
        std::error_code ec;
        bool removed = fs::remove(filePath, ec);
        if (ec) {
            std::cerr << "Error deleting file: " << ec.message() << std::endl;
            return false;
        }
        return removed;
    }

    static std::vector<std::string> listFilesInDirectory(const std::string& dirPath) {
        std::vector<std::string> fileList;
        std::error_code ec;
        if (!fs::exists(dirPath, ec)) {
            std::cerr << "Error: Directory " << dirPath << " does not exist." << std::endl;
            return fileList;
        }
        for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
            if (!ec && entry.is_regular_file()) {
                fileList.push_back(entry.path().filename().string());
            }
        }
        if (ec) {
            std::cerr << "Error reading directory: " << ec.message() << std::endl;
        }
        return fileList;
    }
};