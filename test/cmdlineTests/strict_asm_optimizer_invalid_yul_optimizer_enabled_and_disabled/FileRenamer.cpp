#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void batchRename(const std::string& directoryPath, const std::string& baseName) {
    try {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = file.parent_path() / newFileName;

            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFileName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Batch renaming completed. " << (counter - 1) << " files processed." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }

    std::string dirPath = argv[1];
    std::string baseName = argv[2];

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }

    batchRename(dirPath, baseName);
    return 0;
}