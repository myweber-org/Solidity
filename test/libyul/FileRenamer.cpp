
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& baseName, const std::string& extension)
        : dirPath(directory), newBaseName(baseName), newExtension(extension) {}

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
            std::cerr << "No regular files found in the directory.\n";
            return false;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string newFileName = newBaseName + "_" + std::to_string(counter) + newExtension;
            fs::path newPath = dirPath / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
                return false;
            }
        }

        std::cout << "Successfully renamed " << (counter - 1) << " files.\n";
        return true;
    }

private:
    fs::path dirPath;
    std::string newBaseName;
    std::string newExtension;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory> <base_name> <extension>\n";
        std::cerr << "Example: " << argv[0] << " ./photos image .jpg\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];

    if (extension.empty() || extension[0] != '.') {
        extension = "." + extension;
    }

    FileRenamer renamer(directory, baseName, extension);
    return renamer.renameFiles() ? 0 : 1;
}