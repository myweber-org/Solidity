
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       const std::string& extension) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory.\n";
            return;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
            }
        }

        std::cout << "Renaming completed. " << (counter - 1) << " files processed.\n";
    }
};

int main() {
    std::string dirPath;
    std::string prefix;
    std::string extension;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, dirPath);
    std::cout << "Enter filename prefix: ";
    std::getline(std::cin, prefix);
    std::cout << "Enter file extension (including dot, e.g., .txt): ";
    std::getline(std::cin, extension);

    FileRenamer::renameFilesInDirectory(dirPath, prefix, extension);

    return 0;
}