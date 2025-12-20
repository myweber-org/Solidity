
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath, const std::string& baseName) {
    try {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        int counter = 1;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.path())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();

                std::ostringstream newFilename;
                newFilename << baseName << "_" << std::setw(3) << std::setfill('0') << counter << extension;
                fs::path newPath = oldPath.parent_path() / newFilename.str();

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
                    ++counter;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                }
            }
        }
        std::cout << "Renaming complete. Total files processed: " << (counter - 1) << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string baseName = argv[2];

    renameFilesInDirectory(directoryPath, baseName);
    return 0;
}