
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

        int currentNumber = startNumber;
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = file.parent_path() / newFilename;

            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFilename << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << (currentNumber - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer::renameFilesInDirectory(directoryPath, prefix, startNumber);

    return 0;
}#include <iostream>
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
                    counter++;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
                }
            }
        }
        std::cout << "Renaming completed. Total files processed: " << (counter - 1) << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
}

int main() {
    std::string directory;
    std::string baseName;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);

    renameFilesInDirectory(directory, baseName);
    return 0;
}