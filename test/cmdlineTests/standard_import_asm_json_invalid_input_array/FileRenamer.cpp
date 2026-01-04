
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory, const std::string& baseName) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return;
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(directory)) {
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
        fs::path newPath = directory / newFileName;

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Renaming completed. Total files processed: " << counter - 1 << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string baseName(argv[2]);

    renameFilesSequentially(dirPath, baseName);

    return 0;
}