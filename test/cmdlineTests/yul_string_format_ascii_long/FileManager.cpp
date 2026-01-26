
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
    static bool copyFileWithTimestamp(const std::string& sourcePath, const std::string& destDirectory) {
        if (!fs::exists(sourcePath)) {
            logError("Source file does not exist: " + sourcePath);
            return false;
        }

        if (!fs::exists(destDirectory)) {
            if (!fs::create_directories(destDirectory)) {
                logError("Failed to create destination directory: " + destDirectory);
                return false;
            }
        }

        fs::path sourceFile(sourcePath);
        std::string timestamp = getCurrentTimestamp();
        std::string newFilename = sourceFile.stem().string() + "_" + timestamp + sourceFile.extension().string();
        fs::path destFile = fs::path(destDirectory) / newFilename;

        try {
            fs::copy_file(sourcePath, destFile, fs::copy_options::overwrite_existing);
            logInfo("File copied successfully: " + destFile.string());
            return true;
        } catch (const fs::filesystem_error& e) {
            logError("Failed to copy file: " + std::string(e.what()));
            return false;
        }
    }

    static bool createBackup(const std::string& filePath, const std::string& backupDir = "./backups") {
        return copyFileWithTimestamp(filePath, backupDir);
    }

    static std::string readFileContent(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logError("Cannot open file for reading: " + filePath);
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)), 
                           std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

    static bool writeToFile(const std::string& filePath, const std::string& content, bool append = false) {
        std::ofstream file;
        if (append) {
            file.open(filePath, std::ios::app);
        } else {
            file.open(filePath);
        }

        if (!file.is_open()) {
            logError("Cannot open file for writing: " + filePath);
            return false;
        }

        file << content;
        file.close();
        logInfo("Content written to file: " + filePath);
        return true;
    }

private:
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    static void logInfo(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }

    static void logError(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
};

int main() {
    // Example usage
    FileManager::writeToFile("test.txt", "This is a test file content.");
    
    if (FileManager::createBackup("test.txt")) {
        std::cout << "Backup created successfully." << std::endl;
    }
    
    std::string content = FileManager::readFileContent("test.txt");
    if (!content.empty()) {
        std::cout << "File content: " << content << std::endl;
    }
    
    return 0;
}