
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool writeToFile(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            logError("Failed to open file for writing: " + filePath);
            return false;
        }
        file << content;
        file.close();
        logInfo("Successfully wrote to file: " + filePath);
        return true;
    }

    static std::string readFromFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logError("Failed to open file for reading: " + filePath);
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        logInfo("Successfully read from file: " + filePath);
        return content;
    }

    static bool copyFile(const std::string& sourcePath, const std::string& destPath) {
        try {
            fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
            logInfo("File copied from " + sourcePath + " to " + destPath);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to copy file: " + std::string(e.what()));
            return false;
        }
    }

    static bool deleteFile(const std::string& filePath) {
        if (!fs::exists(filePath)) {
            logWarning("File does not exist: " + filePath);
            return false;
        }
        try {
            fs::remove(filePath);
            logInfo("File deleted: " + filePath);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to delete file: " + std::string(e.what()));
            return false;
        }
    }

    static std::vector<std::string> listFilesInDirectory(const std::string& dirPath) {
        std::vector<std::string> files;
        try {
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
            logInfo("Listed files in directory: " + dirPath);
        } catch (const fs::filesystem_error& e) {
            logError("Failed to list directory: " + std::string(e.what()));
        }
        return files;
    }

    static std::string getFileSize(const std::string& filePath) {
        try {
            auto size = fs::file_size(filePath);
            std::stringstream ss;
            if (size < 1024) {
                ss << size << " bytes";
            } else if (size < 1024 * 1024) {
                ss << std::fixed << std::setprecision(2) << (size / 1024.0) << " KB";
            } else {
                ss << std::fixed << std::setprecision(2) << (size / (1024.0 * 1024.0)) << " MB";
            }
            return ss.str();
        } catch (const fs::filesystem_error& e) {
            logError("Failed to get file size: " + std::string(e.what()));
            return "Unknown";
        }
    }

private:
    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << getCurrentTimestamp() << " - " << message << std::endl;
    }

    static void logWarning(const std::string& message) {
        std::cout << "[WARN] " << getCurrentTimestamp() << " - " << message << std::endl;
    }

    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << getCurrentTimestamp() << " - " << message << std::endl;
    }

    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};