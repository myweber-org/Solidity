#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& baseName, const std::string& extension)
        : dirPath(directory), newBaseName(baseName), fileExtension(extension) {}

    bool renameFiles() {
        std::vector<fs::path> files;
        
        try {
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (fs::is_regular_file(entry.status())) {
                    files.push_back(entry.path());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing directory: " << e.what() << std::endl;
            return false;
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string newFileName = newBaseName + "_" + std::to_string(counter) + fileExtension;
            fs::path newPath = dirPath / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. " << (counter - 1) << " files processed." << std::endl;
        return true;
    }

private:
    fs::path dirPath;
    std::string newBaseName;
    std::string fileExtension;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation .jpg" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];

    if (extension[0] != '.') {
        extension = "." + extension;
    }

    FileRenamer renamer(directory, baseName, extension);
    
    if (!renamer.renameFiles()) {
        return 1;
    }

    return 0;
}