
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& baseName,
                                       const std::string& extension,
                                       int startNumber = 1) {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.path())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return;
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = fs::path(directoryPath) / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. " << (counter - startNumber) << " files processed." << std::endl;
    }
};

int main() {
    std::string directory;
    std::string baseName;
    std::string extension;
    int startNumber;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);
    std::cout << "Enter file extension (including dot): ";
    std::getline(std::cin, extension);
    std::cout << "Enter starting number: ";
    std::cin >> startNumber;

    FileRenamer::renameFilesInDirectory(directory, baseName, extension, startNumber);

    return 0;
}