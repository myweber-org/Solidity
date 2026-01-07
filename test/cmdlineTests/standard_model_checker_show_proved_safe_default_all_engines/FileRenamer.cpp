#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath, const std::string& prefix) {
    try {
        fs::path dir(directoryPath);
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "Error: Invalid directory path.\n";
            return;
        }

        int counter = 0;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry.path())) {
                fs::path oldPath = entry.path();
                fs::path newPath = oldPath.parent_path() / (prefix + oldPath.filename().string());

                try {
                    fs::rename(oldPath, newPath);
                    ++counter;
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << '\n';
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
                }
            }
        }
        std::cout << "Total files renamed: " << counter << '\n';
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>\n";
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string prefix = argv[2];

    renameFilesInDirectory(directoryPath, prefix);
    return 0;
}