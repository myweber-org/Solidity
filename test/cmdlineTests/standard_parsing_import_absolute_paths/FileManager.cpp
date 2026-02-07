#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool copyFileWithTimestamp(const std::string& sourcePath, const std::string& destDir) {
        if (!fs::exists(sourcePath)) {
            logError("Source file does not exist: " + sourcePath);
            return false;
        }

        if (!fs::is_regular_file(sourcePath)) {
            logError("Source path is not a regular file: " + sourcePath);
            return false;
        }

        fs::path sourceFile(sourcePath);
        std::string filename = sourceFile.filename().string();
        std::string timestamp = getCurrentTimestamp();
        
        size_t dotPos = filename.find_last_of('.');
        std::string newFilename;
        
        if (dotPos != std::string::npos) {
            newFilename = filename.substr(0, dotPos) + "_" + timestamp + filename.substr(dotPos);
        } else {
            newFilename = filename + "_" + timestamp;
        }

        fs::path destPath = fs::path(destDir) / newFilename;

        try {
            if (!fs::exists(destDir)) {
                fs::create_directories(destDir);
                logInfo("Created destination directory: " + destDir);
            }

            fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
            logInfo("File copied successfully: " + sourcePath + " -> " + destPath.string());
            
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Filesystem error during copy: " + std::string(e.what()));
            return false;
        } catch (const std::exception& e) {
            logError("Standard exception during copy: " + std::string(e.what()));
            return false;
        }
    }

    static bool deleteOldFiles(const std::string& directory, int daysOld) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            logError("Invalid directory: " + directory);
            return false;
        }

        auto now = fs::file_time_type::clock::now();
        auto cutoffTime = now - std::chrono::hours(24 * daysOld);
        int deletedCount = 0;

        try {
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (fs::is_regular_file(entry.status())) {
                    auto fileTime = fs::last_write_time(entry.path());
                    
                    if (fileTime < cutoffTime) {
                        fs::remove(entry.path());
                        logInfo("Deleted old file: " + entry.path().string());
                        deletedCount++;
                    }
                }
            }
            
            logInfo("Deleted " + std::to_string(deletedCount) + " old files from " + directory);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Filesystem error during deletion: " + std::string(e.what()));
            return false;
        }
    }

    static std::string getFileInfo(const std::string& filePath) {
        if (!fs::exists(filePath)) {
            return "File does not exist: " + filePath;
        }

        try {
            fs::path path(filePath);
            auto fileSize = fs::file_size(path);
            auto lastWrite = fs::last_write_time(path);
            
            std::time_t cftime = std::chrono::system_clock::to_time_t(
                std::chrono::file_clock::to_sys(lastWrite)
            );
            
            std::stringstream ss;
            ss << "File: " << path.filename().string() << "\n"
               << "Size: " << formatFileSize(fileSize) << "\n"
               << "Last modified: " << std::ctime(&cftime)
               << "Path: " << fs::absolute(path).string();
            
            return ss.str();
        } catch (const fs::filesystem_error& e) {
            return "Error getting file info: " + std::string(e.what());
        }
    }

private:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    static std::string formatFileSize(uintmax_t size) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double formattedSize = static_cast<double>(size);
        
        while (formattedSize >= 1024.0 && unitIndex < 4) {
            formattedSize /= 1024.0;
            unitIndex++;
        }
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << formattedSize << " " << units[unitIndex];
        return ss.str();
    }

    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << getCurrentTimestamp() << " - " << message << std::endl;
    }

    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << getCurrentTimestamp() << " - " << message << std::endl;
    }
};