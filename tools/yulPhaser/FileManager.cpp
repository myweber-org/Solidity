#include <iostream>
#include <fstream>
#include <string>
#include <system_error>
#include <filesystem>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool writeToFile(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << " for writing.\n";
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    static std::string readFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << " for reading.\n";
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

    static bool copyFile(const std::string& sourcePath, const std::string& destPath) {
        try {
            fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
            return true;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error copying file: " << e.what() << "\n";
            return false;
        }
    }

    static bool deleteFile(const std::string& filePath) {
        if (!fs::exists(filePath)) {
            std::cerr << "Error: File " << filePath << " does not exist.\n";
            return false;
        }
        return fs::remove(filePath);
    }

    static size_t getFileSize(const std::string& filePath) {
        try {
            return fs::file_size(filePath);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error getting file size: " << e.what() << "\n";
            return 0;
        }
    }
};