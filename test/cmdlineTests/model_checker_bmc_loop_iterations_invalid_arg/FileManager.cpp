
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool fileExists(const std::string& path) {
        return fs::exists(path) && fs::is_regular_file(path);
    }

    static bool directoryExists(const std::string& path) {
        return fs::exists(path) && fs::is_directory(path);
    }

    static bool createDirectory(const std::string& path) {
        if (directoryExists(path)) {
            return true;
        }
        try {
            return fs::create_directories(path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return false;
        }
    }

    static uintmax_t getFileSize(const std::string& path) {
        if (!fileExists(path)) {
            return 0;
        }
        try {
            return fs::file_size(path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error getting file size: " << e.what() << std::endl;
            return 0;
        }
    }
};