#include <iostream>
#include <fstream>
#include <string>
#include <system_error>
#include <filesystem>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool writeToFile(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            logError("Failed to open file for writing: " + filePath);
            return false;
        }
        file << content;
        if (file.fail()) {
            logError("Failed to write content to file: " + filePath);
            file.close();
            return false;
        }
        file.close();
        logInfo("Successfully wrote to file: " + filePath);
        return true;
    }

    static std::string readFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::in);
        if (!file.is_open()) {
            logError("Failed to open file for reading: " + filePath);
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        if (file.fail() && !file.eof()) {
            logError("Error occurred while reading file: " + filePath);
            file.close();
            return "";
        }
        file.close();
        logInfo("Successfully read from file: " + filePath);
        return content;
    }

    static bool copyFile(const std::string& sourcePath, const std::string& destPath) {
        std::error_code ec;
        fs::copy(sourcePath, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            logError("Failed to copy file from " + sourcePath + " to " + destPath + ": " + ec.message());
            return false;
        }
        logInfo("Successfully copied file from " + sourcePath + " to " + destPath);
        return true;
    }

    static bool deleteFile(const std::string& filePath) {
        std::error_code ec;
        if (!fs::remove(filePath, ec)) {
            logError("Failed to delete file: " + filePath + " - " + ec.message());
            return false;
        }
        logInfo("Successfully deleted file: " + filePath);
        return true;
    }

    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath);
    }

    static uintmax_t getFileSize(const std::string& filePath) {
        std::error_code ec;
        auto size = fs::file_size(filePath, ec);
        if (ec) {
            logError("Failed to get file size for: " + filePath + " - " + ec.message());
            return 0;
        }
        return size;
    }

private:
    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }

    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }
};