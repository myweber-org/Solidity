
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory, const std::string& baseName) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return;
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry.path());
        }
    }

    if (files.empty()) {
        std::cout << "No files found in the directory." << std::endl;
        return;
    }

    std::sort(files.begin(), files.end());

    int counter = 1;
    for (const auto& oldPath : files) {
        std::string extension = oldPath.extension().string();
        std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = directory / newFileName;

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Renaming completed. Total files processed: " << counter - 1 << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string baseName(argv[2]);

    renameFilesSequentially(dirPath, baseName);

    return 0;
}
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix)
        : targetDirectory(directory), namePrefix(prefix) {}

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
            std::cout << "No files found in the directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFilename = namePrefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << std::endl;
        return true;
    }

private:
    std::string targetDirectory;
    std::string namePrefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(directory, prefix);
    return renamer.renameFiles() ? 0 : 1;
}