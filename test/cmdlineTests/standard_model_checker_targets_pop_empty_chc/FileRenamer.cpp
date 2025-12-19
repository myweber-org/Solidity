
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static bool renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& prefix,
                                       int startNumber = 1,
                                       const std::string& targetExtension = "") {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.path())) {
                if (targetExtension.empty() ||
                    entry.path().extension().string() == targetExtension) {
                    files.push_back(entry.path());
                }
            }
        }

        if (files.empty()) {
            std::cout << "No matching files found." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << (counter - startNumber) << " files." << std::endl;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [startNumber] [extension]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1 .jpg" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extension = (argc > 4) ? argv[4] : "";

    if (!extension.empty() && extension[0] != '.') {
        extension = "." + extension;
    }

    return FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extension) ? 0 : 1;
}