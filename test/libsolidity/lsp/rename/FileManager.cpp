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
    
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }
    
    static bool createDirectory(const std::string& dirPath) {
        try {
            if (fs::exists(dirPath)) {
                return fs::is_directory(dirPath);
            }
            return fs::create_directories(dirPath);
        } catch (const fs::filesystem_error& e) {
            logError("Failed to create directory: " + std::string(e.what()));
            return false;
        }
    }
    
    static uintmax_t getFileSize(const std::string& filePath) {
        try {
            if (fileExists(filePath)) {
                return fs::file_size(filePath);
            }
        } catch (const fs::filesystem_error& e) {
            logError("Failed to get file size: " + std::string(e.what()));
        }
        return 0;
    }

private:
    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
    
    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }
};