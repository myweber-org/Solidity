#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory, const std::string& baseName, const std::string& extension) {
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
        std::cout << "No files found in the directory.\n";
        return;
    }

    std::sort(files.begin(), files.end());

    int counter = 1;
    for (const auto& oldPath : files) {
        std::stringstream newFilename;
        newFilename << baseName << std::setw(4) << std::setfill('0') << counter << extension;
        fs::path newPath = directory / newFilename.str();

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << '\n';
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
        }
    }

    std::cout << "Renaming completed. " << (counter - 1) << " files processed.\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name> <extension>\n";
        std::cerr << "Example: " << argv[0] << " ./photos vacation .jpg\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string baseName(argv[2]);
    std::string extension(argv[3]);

    renameFilesSequentially(targetDir, baseName, extension);

    return 0;
}