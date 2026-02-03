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
        if (!fs::exists(filePath)) {
            logError("File does not exist: " + filePath);
            return "";
        }
        
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
    
    static bool copyFile(const std::string& sourcePath, const std::string& destinationPath) {
        try {
            fs::copy_file(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
            logInfo("Successfully copied file from " + sourcePath + " to " + destinationPath);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("File copy failed: " + std::string(e.what()));
            return false;
        }
    }
    
    static bool deleteFile(const std::string& filePath) {
        if (!fs::exists(filePath)) {
            logWarning("File does not exist, nothing to delete: " + filePath);
            return true;
        }
        
        try {
            if (fs::remove(filePath)) {
                logInfo("Successfully deleted file: " + filePath);
                return true;
            } else {
                logError("Failed to delete file: " + filePath);
                return false;
            }
        } catch (const fs::filesystem_error& e) {
            logError("File deletion failed: " + std::string(e.what()));
            return false;
        }
    }
    
    static bool createDirectory(const std::string& dirPath) {
        try {
            if (fs::create_directories(dirPath)) {
                logInfo("Successfully created directory: " + dirPath);
                return true;
            }
            return false;
        } catch (const fs::filesystem_error& e) {
            logError("Directory creation failed: " + std::string(e.what()));
            return false;
        }
    }
    
    static size_t getFileSize(const std::string& filePath) {
        try {
            if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
                return fs::file_size(filePath);
            }
            logError("Cannot get size for non-existent or non-regular file: " + filePath);
            return 0;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to get file size: " + std::string(e.what()));
            return 0;
        }
    }

private:
    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
    
    static void logWarning(const std::string& message) {
        std::cout << "[WARNING] " << message << std::endl;
    }
    
    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }
};