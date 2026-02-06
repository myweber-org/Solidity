#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath) {
    try {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFileName = std::to_string(counter) + "_" + oldPath.stem().string() + extension;
            fs::path newPath = oldPath.parent_path() / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. Processed " << (counter - 1) << " files." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string targetDirectory = argv[1];
    if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
        std::cerr << "Error: Invalid directory path provided." << std::endl;
        return 1;
    }

    renameFilesInDirectory(targetDirectory);
    return 0;
}#include <iostream>
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

    fs::path directory_path = argv[1];
    std::string base_name = argv[2];

    if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();

            std::ostringstream new_filename;
            new_filename << base_name << "_"
                         << std::setw(4) << std::setfill('0') << counter
                         << extension;
            fs::path new_path = old_path.parent_path() / new_filename.str();

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename()
                          << " -> " << new_path.filename() << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << old_path.filename()
                          << ": " << e.what() << '\n';
            }
        }
    }

    std::cout << "File renaming completed. Total files processed: "
              << counter - 1 << '\n';
    return 0;
}