
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath, const std::string& baseName) {
    try {
        fs::path dirPath(directoryPath);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "Error: Invalid or non-existent directory path.\n";
            return;
        }

        int counter = 1;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.status())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();

                std::ostringstream newFilename;
                newFilename << baseName << "_" << std::setw(3) << std::setfill('0') << counter << extension;
                fs::path newPath = oldPath.parent_path() / newFilename.str();

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << '\n';
                    ++counter;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
                }
            }
        }
        std::cout << "Renaming complete. " << (counter - 1) << " files processed.\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
}

int main() {
    std::string dir, base;
    std::cout << "Enter directory path: ";
    std::getline(std::cin, dir);
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, base);

    renameFilesInDirectory(dir, base);
    return 0;
}