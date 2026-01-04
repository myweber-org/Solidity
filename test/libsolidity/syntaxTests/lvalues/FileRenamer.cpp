#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath) {
    try {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFileName = std::to_string(counter) + "_" + oldPath.stem().string() + extension;
            fs::path newPath = oldPath.parent_path() / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. Processed " << (counter - 1) << " files." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string targetDirectory = argv[1];
    if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
        std::cerr << "Error: Invalid directory path provided." << std::endl;
        return 1;
    }

    renameFilesInDirectory(targetDirectory);
    return 0;
}