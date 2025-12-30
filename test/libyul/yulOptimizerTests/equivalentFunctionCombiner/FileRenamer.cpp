
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void batchRename(const std::string& directoryPath) {
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
            std::string newName = directoryPath + "/" + std::to_string(counter) + extension;
            
            try {
                fs::rename(file, newName);
                std::cout << "Renamed: " << file.filename() << " -> " << counter << extension << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Batch renaming completed. Processed " << (counter - 1) << " files." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string path = argv[1];
    if (!fs::exists(path) || !fs::is_directory(path)) {
        std::cerr << "Invalid directory path: " << path << std::endl;
        return 1;
    }

    batchRename(path);
    return 0;
}