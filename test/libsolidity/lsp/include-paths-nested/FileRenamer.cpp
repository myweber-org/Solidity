
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static bool renameFilesInDirectory(const fs::path& directory, 
                                       const std::string& prefix, 
                                       int startNumber = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end(), 
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int currentNumber = startNumber;
        bool allRenamed = true;

        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();
            
            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() 
                          << " -> " << newFilename << std::endl;
                currentNumber++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() 
                          << ": " << e.what() << std::endl;
                allRenamed = false;
            }
        }

        if (allRenamed) {
            std::cout << "All files renamed successfully." << std::endl;
        } else {
            std::cout << "Some files could not be renamed." << std::endl;
        }

        return allRenamed;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1" << std::endl;
        return 1;
    }

    fs::path directory(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = 1;

    if (argc >= 4) {
        try {
            startNumber = std::stoi(argv[3]);
            if (startNumber < 0) {
                std::cerr << "Start number must be non-negative." << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid start number: " << e.what() << std::endl;
            return 1;
        }
    }

    return FileRenamer::renameFilesInDirectory(directory, prefix, startNumber) ? 0 : 1;
}