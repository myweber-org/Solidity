
#include <iostream>
#include <fstream>
#include <string>
#include <system_error>
#include <filesystem>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool writeToFile(const std::string& filepath, const std::string& content) {
        std::ofstream file(filepath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            logError("Failed to open file for writing: " + filepath);
            return false;
        }
        
        file << content;
        if (file.fail()) {
            logError("Failed to write content to file: " + filepath);
            file.close();
            return false;
        }
        
        file.close();
        logInfo("Successfully wrote to file: " + filepath);
        return true;
    }
    
    static std::string readFromFile(const std::string& filepath) {
        if (!fs::exists(filepath)) {
            logError("File does not exist: " + filepath);
            return "";
        }
        
        std::ifstream file(filepath, std::ios::in);
        if (!file.is_open()) {
            logError("Failed to open file for reading: " + filepath);
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        file.close();
        logInfo("Successfully read from file: " + filepath);
        return content;
    }
    
    static bool copyFile(const std::string& source, const std::string& destination) {
        try {
            fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
            logInfo("Successfully copied file from " + source + " to " + destination);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to copy file: " + std::string(e.what()));
            return false;
        }
    }
    
    static bool deleteFile(const std::string& filepath) {
        if (!fs::exists(filepath)) {
            logWarning("File does not exist, nothing to delete: " + filepath);
            return true;
        }
        
        try {
            if (fs::remove(filepath)) {
                logInfo("Successfully deleted file: " + filepath);
                return true;
            } else {
                logError("Failed to delete file: " + filepath);
                return false;
            }
        } catch (const fs::filesystem_error& e) {
            logError("Exception while deleting file: " + std::string(e.what()));
            return false;
        }
    }
    
    static size_t getFileSize(const std::string& filepath) {
        try {
            if (fs::exists(filepath)) {
                return fs::file_size(filepath);
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
    
    static void logWarning(const std::string& message) {
        std::cout << "[WARNING] " << message << std::endl;
    }
};