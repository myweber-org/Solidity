
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix, int startNumber = 1)
        : targetDirectory(directory), filePrefix(prefix), counter(startNumber) {}

    bool renameFiles() {
        if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(targetDirectory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFilename = filePrefix + "_" + std::to_string(counter) + extension;
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

        std::cout << "Renaming completed successfully. Total files processed: " << files.size() << std::endl;
        return true;
    }

private:
    std::string targetDirectory;
    std::string filePrefix;
    int counter;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <file_prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation 1" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory, prefix, startNumber);
    return renamer.renameFiles() ? 0 : 1;
}