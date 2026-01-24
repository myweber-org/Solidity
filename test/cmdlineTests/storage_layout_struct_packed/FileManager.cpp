
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool copyFileWithTimestamp(const std::string& sourcePath, const std::string& destinationDir) {
        if (!fs::exists(sourcePath)) {
            logError("Source file does not exist: " + sourcePath);
            return false;
        }

        if (!fs::exists(destinationDir)) {
            if (!fs::create_directories(destinationDir)) {
                logError("Failed to create destination directory: " + destinationDir);
                return false;
            }
        }

        fs::path sourceFile(sourcePath);
        std::string timestamp = getCurrentTimestamp();
        std::string newFilename = sourceFile.stem().string() + "_" + timestamp + sourceFile.extension().string();
        fs::path destinationFile = fs::path(destinationDir) / newFilename;

        try {
            fs::copy_file(sourcePath, destinationFile, fs::copy_options::overwrite_existing);
            logInfo("File copied successfully: " + destinationFile.string());
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to copy file: " + std::string(e.what()));
            return false;
        }
    }

    static bool deleteOldFiles(const std::string& directoryPath, int daysOld) {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            logError("Invalid directory: " + directoryPath);
            return false;
        }

        auto now = fs::file_time_type::clock::now();
        auto cutoffTime = now - std::chrono::hours(24 * daysOld);
        int deletedCount = 0;

        try {
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                if (fs::is_regular_file(entry.status())) {
                    auto lastWriteTime = fs::last_write_time(entry.path());
                    if (lastWriteTime < cutoffTime) {
                        fs::remove(entry.path());
                        deletedCount++;
                        logInfo("Deleted old file: " + entry.path().string());
                    }
                }
            }
            logInfo("Deleted " + std::to_string(deletedCount) + " old files from " + directoryPath);
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Error deleting old files: " + std::string(e.what()));
            return false;
        }
    }

    static std::string getFileSize(const std::string& filePath) {
        if (!fs::exists(filePath)) {
            return "File not found";
        }

        uintmax_t size = fs::file_size(filePath);
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unitIndex = 0;
        double displaySize = static_cast<double>(size);

        while (displaySize >= 1024 && unitIndex < 3) {
            displaySize /= 1024;
            unitIndex++;
        }

        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << displaySize << " " << units[unitIndex];
        return stream.str();
    }

private:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        
        std::ostringstream stream;
        stream << std::put_time(&tm, "%Y%m%d_%H%M%S");
        return stream.str();
    }

    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }

    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
};