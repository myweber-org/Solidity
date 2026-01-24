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
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newFileName = directoryPath + "/" + std::to_string(counter) + "_" + file.stem().string() + extension;
            fs::path newPath = newFileName;

            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newPath.filename() << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. Processed " << (counter - 1) << " files." << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string dirPath = argv[1];
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error: Invalid directory path provided." << std::endl;
        return 1;
    }

    renameFilesInDirectory(dirPath);
    return 0;
}