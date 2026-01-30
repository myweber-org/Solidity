
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& baseName) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return;
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
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }
    }
    std::cout << "Renaming complete. Total files processed: " << (counter - 1) << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string baseName(argv[2]);

    renameFilesInDirectory(dirPath, baseName);
    return 0;
}
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& prefix,
                                       int startNumber = 1) {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = file.parent_path() / newFilename;

            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. " << (counter - startNumber) << " files processed." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer::renameFilesInDirectory(directoryPath, prefix, startNumber);
    return 0;
}