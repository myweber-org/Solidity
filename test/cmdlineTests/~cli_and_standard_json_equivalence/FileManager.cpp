
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool copyFileWithTimestamp(const std::string& sourcePath, const std::string& destDir) {
        if (!fs::exists(sourcePath)) {
            logError("Source file does not exist: " + sourcePath);
            return false;
        }

        if (!fs::exists(destDir)) {
            if (!fs::create_directories(destDir)) {
                logError("Failed to create destination directory: " + destDir);
                return false;
            }
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        fs::path sourceFile(sourcePath);
        std::string newFilename = sourceFile.stem().string() + 
                                 "_" + std::to_string(timestamp) + 
                                 sourceFile.extension().string();
        
        fs::path destFile = fs::path(destDir) / newFilename;

        try {
            fs::copy_file(sourcePath, destFile, fs::copy_options::overwrite_existing);
            logInfo("File copied successfully: " + destFile.string());
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to copy file: " + std::string(e.what()));
            return false;
        }
    }

    static bool validateFileExtension(const std::string& filePath, const std::vector<std::string>& allowedExtensions) {
        fs::path path(filePath);
        std::string extension = path.extension().string();
        
        if (extension.empty()) {
            return false;
        }

        for (const auto& ext : allowedExtensions) {
            if (extension == ext) {
                return true;
            }
        }
        
        return false;
    }

    static uintmax_t getFileSize(const std::string& filePath) {
        try {
            if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
                return fs::file_size(filePath);
            }
        } catch (const fs::filesystem_error& e) {
            logError("Error getting file size: " + std::string(e.what()));
        }
        return 0;
    }

private:
    static void logInfo(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::cout << "[INFO][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") 
                  << "] " << message << std::endl;
    }

    static void logError(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::cerr << "[ERROR][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") 
                  << "] " << message << std::endl;
    }
};