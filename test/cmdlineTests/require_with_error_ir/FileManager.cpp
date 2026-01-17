
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }

    static bool directoryExists(const std::string& dirPath) {
        return fs::exists(dirPath) && fs::is_directory(dirPath);
    }

    static bool createDirectory(const std::string& dirPath) {
        if (directoryExists(dirPath)) {
            return true;
        }
        try {
            return fs::create_directories(dirPath);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return false;
        }
    }

    static void displayFileInfo(const std::string& path) {
        if (!fs::exists(path)) {
            std::cout << "Path does not exist: " << path << std::endl;
            return;
        }

        try {
            auto status = fs::status(path);
            auto perms = status.permissions();

            std::cout << "Path: " << path << std::endl;
            std::cout << "Type: " << (fs::is_directory(path) ? "Directory" : "File") << std::endl;
            std::cout << "Size: " << (fs::is_regular_file(path) ? std::to_string(fs::file_size(path)) : "N/A") << " bytes" << std::endl;
            std::cout << "Permissions: ";
            std::cout << ((perms & fs::perms::owner_read) != fs::perms::none ? "r" : "-");
            std::cout << ((perms & fs::perms::owner_write) != fs::perms::none ? "w" : "-");
            std::cout << ((perms & fs::perms::owner_exec) != fs::perms::none ? "x" : "-");
            std::cout << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing file info: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::string testFile = "test.txt";
    std::string testDir = "test_directory";

    std::cout << "Testing FileManager functions:" << std::endl;
    std::cout << "==============================" << std::endl;

    bool fileExist = FileManager::fileExists(testFile);
    std::cout << "File '" << testFile << "' exists: " << (fileExist ? "Yes" : "No") << std::endl;

    bool dirCreated = FileManager::createDirectory(testDir);
    std::cout << "Directory '" << testDir << "' created: " << (dirCreated ? "Yes" : "No") << std::endl;

    if (FileManager::directoryExists(testDir)) {
        std::cout << "\nDirectory info:" << std::endl;
        FileManager::displayFileInfo(testDir);
    }

    return 0;
}