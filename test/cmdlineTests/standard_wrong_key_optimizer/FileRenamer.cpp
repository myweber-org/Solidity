
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& prefix,
                                       int startNumber = 1) {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& filePath : files) {
            std::string extension = filePath.extension().string();
            std::string newFileName = prefix + std::to_string(currentNumber) + extension;
            fs::path newFilePath = filePath.parent_path() / newFileName;

            try {
                fs::rename(filePath, newFilePath);
                std::cout << "Renamed: " << filePath.filename() << " -> " << newFileName << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << filePath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Processed " << (currentNumber - startNumber) << " files." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc >= 4) ? std::stoi(argv[3]) : 1;

    FileRenamer::renameFilesInDirectory(directoryPath, prefix, startNumber);

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

void renameFileWithTimestamp(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "Error: File does not exist.\n";
        return;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file.\n";
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);

    std::ostringstream timestampStream;
    timestampStream << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    std::string timestamp = timestampStream.str();

    fs::path parentDir = filePath.parent_path();
    std::string originalName = filePath.filename().string();
    fs::path newPath = parentDir / (timestamp + "_" + originalName);

    try {
        fs::rename(filePath, newPath);
        std::cout << "Renamed: " << originalName << " -> " << newPath.filename().string() << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>\n";
        return 1;
    }

    fs::path targetFile(argv[1]);
    renameFileWithTimestamp(targetFile);

    return 0;
}