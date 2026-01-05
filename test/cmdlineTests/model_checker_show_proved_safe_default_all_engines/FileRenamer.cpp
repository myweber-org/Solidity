
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directoryPath, const std::string& newPrefix)
        : dirPath(directoryPath), prefix(newPrefix) {}

    bool renameFiles() {
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = 1;
        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();
            std::string newFilename = prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming process completed." << std::endl;
        return true;
    }

private:
    std::string dirPath;
    std::string prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <new_prefix>" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(directory, prefix);
    if (!renamer.renameFiles()) {
        return 1;
    }

    return 0;
}