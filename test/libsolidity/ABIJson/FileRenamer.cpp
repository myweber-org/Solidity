
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& prefix,
                                       int startNumber = 1,
                                       const std::string& targetExtension = "") {
        try {
            if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
                std::cerr << "Error: Invalid directory path." << std::endl;
                return;
            }

            std::vector<fs::path> files;
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                if (fs::is_regular_file(entry.path())) {
                    if (targetExtension.empty() ||
                        entry.path().extension() == targetExtension) {
                        files.push_back(entry.path());
                    }
                }
            }

            std::sort(files.begin(), files.end());

            int counter = startNumber;
            for (const auto& file : files) {
                std::string newFilename = prefix + std::to_string(counter) + file.extension().string();
                fs::path newPath = file.parent_path() / newFilename;

                try {
                    fs::rename(file, newPath);
                    std::cout << "Renamed: " << file.filename() << " -> " << newFilename << std::endl;
                    counter++;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << file.filename() << ": " << e.what() << std::endl;
                }
            }

            std::cout << "Renaming completed. Total files processed: " << (counter - startNumber) << std::endl;

        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::string directory;
    std::string prefix;
    std::string extension;
    int startNumber;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);

    std::cout << "Enter filename prefix: ";
    std::getline(std::cin, prefix);

    std::cout << "Enter file extension to filter (leave empty for all files): ";
    std::getline(std::cin, extension);

    std::cout << "Enter starting number: ";
    std::cin >> startNumber;

    if (!extension.empty() && extension[0] != '.') {
        extension = "." + extension;
    }

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extension);

    return 0;
}