
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool fileExists(const std::string& filePath) {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }

    static long long getFileSize(const std::string& filePath) {
        if (!fileExists(filePath)) {
            return -1;
        }
        try {
            return fs::file_size(filePath);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            return -1;
        }
    }

    static bool validateFile(const std::string& filePath, long long maxSizeBytes) {
        if (!fileExists(filePath)) {
            std::cerr << "File does not exist: " << filePath << std::endl;
            return false;
        }

        long long size = getFileSize(filePath);
        if (size < 0) {
            std::cerr << "Unable to determine file size." << std::endl;
            return false;
        }

        if (size > maxSizeBytes) {
            std::cerr << "File size exceeds limit. Size: " << size
                      << " bytes, Limit: " << maxSizeBytes << " bytes." << std::endl;
            return false;
        }

        std::cout << "File validation successful: " << filePath
                  << " (Size: " << size << " bytes)" << std::endl;
        return true;
    }
};

int main() {
    std::string testFile = "test_data.txt";
    const long long MAX_SIZE = 1024 * 1024;

    bool isValid = FileManager::validateFile(testFile, MAX_SIZE);
    if (isValid) {
        std::cout << "File is ready for processing." << std::endl;
    } else {
        std::cout << "File validation failed." << std::endl;
    }

    return 0;
}