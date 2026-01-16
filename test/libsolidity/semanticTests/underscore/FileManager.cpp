
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
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        logInfo("Successfully read from file: " + filePath);
        return content;
    }

    static bool copyFile(const std::string& sourcePath, const std::string& destinationPath) {
        try {
            fs::copy_file(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
            logInfo("Successfully copied file from " + sourcePath + " to " + destinationPath);
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
            logInfo("Successfully deleted file: " + filePath);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to delete file: " + std::string(e.what()));
            return false;
        }
    }

    static std::vector<std::string> listFilesInDirectory(const std::string& directoryPath) {
        std::vector<std::string> files;
        try {
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
            logInfo("Successfully listed files in directory: " + directoryPath);
        } catch (const fs::filesystem_error& e) {
            logError("Failed to list files in directory: " + std::string(e.what()));
        }
        return files;
    }

    static bool createDirectory(const std::string& directoryPath) {
        try {
            if (fs::exists(directoryPath)) {
                logWarning("Directory already exists: " + directoryPath);
                return false;
            }
            fs::create_directory(directoryPath);
            logInfo("Successfully created directory: " + directoryPath);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to create directory: " + std::string(e.what()));
            return false;
        }
    }

private:
    static void logInfo(const std::string& message) {
        logMessage("INFO", message);
    }

    static void logWarning(const std::string& message) {
        logMessage("WARNING", message);
    }

    static void logError(const std::string& message) {
        logMessage("ERROR", message);
    }

    static void logMessage(const std::string& level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

        std::cout << "[" << ss.str() << "] [" << level << "] " << message << std::endl;
    }
};