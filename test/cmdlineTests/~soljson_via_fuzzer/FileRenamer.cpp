#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& baseName)
        : dirPath(directory), newBaseName(baseName) {}

    bool renameFiles() {
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFileName = newBaseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << "\n";
        return true;
    }

private:
    std::string dirPath;
    std::string newBaseName;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <new_base_name>\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string baseName = argv[2];

    FileRenamer renamer(directory, baseName);
    if (!renamer.renameFiles()) {
        return 1;
    }

    return 0;
}