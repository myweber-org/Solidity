
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>\n";
        return 1;
    }

    fs::path directory = argv[1];
    std::string baseName = argv[2];

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path oldPath = entry.path();
            std::string extension = oldPath.extension().string();

            std::ostringstream newFilename;
            newFilename << baseName << "_" << std::setw(4) << std::setfill('0') << counter << extension;
            fs::path newPath = directory / newFilename.str();

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << "\n";
            }
        }
    }

    std::cout << "File renaming completed. Total files processed: " << (counter - 1) << "\n";
    return 0;
}