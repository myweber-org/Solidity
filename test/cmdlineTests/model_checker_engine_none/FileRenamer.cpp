#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath, const std::string& baseName) {
    try {
        fs::path dirPath(directoryPath);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "Error: Invalid directory path.\n";
            return;
        }

        int counter = 1;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.path())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();

                std::stringstream newFilename;
                newFilename << baseName << "_" << std::setw(4) << std::setfill('0') << counter << extension;
                fs::path newPath = oldPath.parent_path() / newFilename.str();

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << "\n";
                    ++counter;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
                }
            }
        }
        std::cout << "Renaming complete. " << (counter - 1) << " files processed.\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation\n";
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string baseName = argv[2];

    renameFilesInDirectory(directoryPath, baseName);
    return 0;
}