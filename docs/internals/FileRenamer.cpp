
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static bool renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       const std::string& extension,
                                       int startNumber = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << (currentNumber - startNumber) << " files." << std::endl;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> <extension> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos image_ .jpg 1" << std::endl;
        return 1;
    }

    fs::path directory = argv[1];
    std::string prefix = argv[2];
    std::string extension = argv[3];
    int startNumber = (argc > 4) ? std::stoi(argv[4]) : 1;

    return FileRenamer::renameFilesInDirectory(directory, prefix, extension, startNumber) ? 0 : 1;
}