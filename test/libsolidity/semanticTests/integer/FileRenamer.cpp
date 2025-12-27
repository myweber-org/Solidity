
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesSequentially(const std::string& directoryPath, const std::string& baseName) {
    try {
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
            std::cout << "No files found in the directory." << std::endl;
            return;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main() {
    std::string dirPath;
    std::string baseName;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, dirPath);
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);

    renameFilesSequentially(dirPath, baseName);

    return 0;
}