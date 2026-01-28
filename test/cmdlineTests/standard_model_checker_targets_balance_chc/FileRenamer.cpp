
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void batchRename(const std::string& directoryPath, const std::string& baseName) {
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
        std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = oldPath.parent_path() / newFileName;

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Renaming complete. " << (counter - 1) << " files processed." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }

    std::string dirPath = argv[1];
    std::string baseName = argv[2];

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }

    batchRename(dirPath, baseName);
    return 0;
}
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
        if (fs::is_regular_file(entry.status())) {
            fs::path oldPath = entry.path();
            std::string extension = oldPath.extension().string();

            std::ostringstream newFilename;
            newFilename << baseName << "_"
                        << std::setw(4) << std::setfill('0') << counter
                        << extension;

            fs::path newPath = oldPath.parent_path() / newFilename.str();

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename()
                          << " -> " << newPath.filename() << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename()
                          << ": " << e.what() << '\n';
            }
        }
    }

    std::cout << "Renaming completed. Total files processed: "
              << (counter - 1) << '\n';
    return 0;
}