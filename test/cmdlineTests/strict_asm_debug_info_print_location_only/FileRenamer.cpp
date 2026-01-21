
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory, const std::string& baseName) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = directory / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. " << (counter - 1) << " files processed." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string baseName(argv[2]);

    FileRenamer::renameFilesInDirectory(targetDir, baseName);

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
    static bool renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       int startNumber = 1,
                                       const std::string& extensionFilter = "") {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                if (extensionFilter.empty() ||
                    entry.path().extension().string() == extensionFilter) {
                    files.push_back(entry);
                }
            }
        }

        if (files.empty()) {
            std::cout << "No files found matching criteria." << std::endl;
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
            std::string newFilename = prefix + std::to_string(currentNumber) + oldPath.extension().string();
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                allRenamed = false;
            }
        }

        return allRenamed;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [startNumber] [extensionFilter]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1 .jpg" << std::endl;
        return 1;
    }

    fs::path directory(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extensionFilter = (argc > 4) ? argv[4] : "";

    if (!extensionFilter.empty() && extensionFilter[0] != '.') {
        extensionFilter = "." + extensionFilter;
    }

    bool success = FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extensionFilter);
    
    return success ? 0 : 1;
}