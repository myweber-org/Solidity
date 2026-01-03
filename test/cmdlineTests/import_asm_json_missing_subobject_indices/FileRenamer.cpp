
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
                                       int startNumber = 1,
                                       const std::string& targetExtension = "") {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                if (targetExtension.empty() ||
                    entry.path().extension().string() == targetExtension) {
                    files.push_back(entry.path());
                }
            }
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << (counter - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [startNumber] [extension]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./images photo_ 1 .jpg" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extension = (argc > 4) ? argv[4] : "";

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extension);

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
        std::cerr << "Error: File does not exist." << std::endl;
        return;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestamp;
    timestamp << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");

    fs::path parentDir = filePath.parent_path();
    fs::path newFileName = parentDir / (timestamp.str() + "_" + filePath.filename().string());

    try {
        fs::rename(filePath, newFileName);
        std::cout << "Renamed: " << filePath.filename() << " -> " << newFileName.filename() << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    fs::path targetFile(argv[1]);
    renameFileWithTimestamp(targetFile);

    return 0;
}