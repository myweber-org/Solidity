#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& baseName) {
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
                std::cout << "Renamed: " << entry.path().filename() << " -> " << newFileName << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << entry.path().filename() << ": " << e.what() << '\n';
            }
        }
    }
    std::cout << "Renaming complete. Total files processed: " << (counter - 1) << '\n';
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>\n";
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string baseName(argv[2]);

    renameFilesInDirectory(dirPath, baseName);
    return 0;
}