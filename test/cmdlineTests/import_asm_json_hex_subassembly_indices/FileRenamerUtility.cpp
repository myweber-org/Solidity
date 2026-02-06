
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
                                       const std::string& targetExtension = "") {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        bool hasError = false;

        for (const auto& oldPath : files) {
            std::string extension = targetExtension.empty() 
                ? oldPath.extension().string() 
                : (targetExtension[0] == '.' ? targetExtension : "." + targetExtension);

            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = directory / newFilename;

            try {
                if (oldPath != newPath) {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << "\n";
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
                hasError = true;
            }

            ++currentNumber;
        }

        if (!hasError) {
            std::cout << "Batch renaming completed successfully.\n";
        }
        return !hasError;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number] [target_extension]\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1 jpg\n";
        return 1;
    }

    fs::path dirPath = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extension = (argc > 4) ? argv[4] : "";

    bool success = FileRenamer::renameFilesInDirectory(dirPath, prefix, startNumber, extension);
    return success ? 0 : 1;
}