
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

void renameFilesInDirectory(const std::string& directoryPath, const std::string& baseName) {
    try {
        fs::path dir(directoryPath);
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        int counter = 1;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry.status())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();

                std::ostringstream newFilename;
                newFilename << baseName << "_" << std::setw(4) << std::setfill('0') << counter << extension;
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

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int currentNumber = startNumber;
        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();

            std::string newFilename = prefix + "_" + std::to_string(currentNumber) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. " << (currentNumber - startNumber) << " files processed." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation 1" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc >= 4) ? std::stoi(argv[3]) : 1;

    FileRenamer::renameFilesInDirectory(directoryPath, prefix, startNumber);

    return 0;
}