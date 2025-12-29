
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
                                       int startNumber = 1,
                                       const std::string& extensionFilter = "") {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                if (extensionFilter.empty() ||
                    entry.path().extension() == extensionFilter) {
                    files.push_back(entry.path());
                }
            }
        }

        if (files.empty()) {
            std::cout << "No files found matching criteria." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(currentNumber) + oldPath.extension().string();
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

        std::cout << "Successfully renamed " << files.size() << " files." << std::endl;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [startNumber] [extensionFilter]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./images photo_ 1 .jpg" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extensionFilter = (argc > 4) ? argv[4] : "";

    if (!extensionFilter.empty() && extensionFilter[0] != '.') {
        extensionFilter = "." + extensionFilter;
    }

    bool success = FileRenamer::renameFilesInDirectory(targetDir, prefix, startNumber, extensionFilter);
    return success ? 0 : 1;
}