
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory, const std::string& baseName) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            std::string extension = entry.path().extension().string();
            std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = directory / newFileName;

            try {
                fs::rename(entry.path(), newPath);
                std::cout << "Renamed: " << entry.path().filename() << " -> " << newFileName << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << entry.path().filename() << ": " << e.what() << "\n";
            }
        }
    }

    std::cout << "Renaming complete. Total files processed: " << (counter - 1) << "\n";
}

int main() {
    std::string dirPath, baseName;
    
    std::cout << "Enter directory path: ";
    std::getline(std::cin, dirPath);
    
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);

    renameFilesSequentially(dirPath, baseName);
    
    return 0;
}