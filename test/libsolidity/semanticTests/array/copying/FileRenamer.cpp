#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix)
        : dir_path(directory), file_prefix(prefix) {}

    bool renameFiles() {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No regular files found in the directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = 1;
        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = file_prefix + "_" + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming process completed. Total files processed: " << counter - 1 << std::endl;
        return true;
    }

private:
    std::string dir_path;
    std::string file_prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <file_prefix>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(directory, prefix);
    if (!renamer.renameFiles()) {
        return 1;
    }

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

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                if (extensionFilter.empty() ||
                    entry.path().extension() == extensionFilter) {
                    files.push_back(entry.path());
                }
            }
        }

        if (files.empty()) {
            std::cout << "No files found matching criteria." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(currentNumber) + oldPath.extension().string();
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << files.size() << " files." << std::endl;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [startNumber] [extensionFilter]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./images photo_ 1 .jpg" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extensionFilter = (argc > 4) ? argv[4] : "";

    if (!extensionFilter.empty() && extensionFilter[0] != '.') {
        extensionFilter = "." + extensionFilter;
    }

    bool success = FileRenamer::renameFilesInDirectory(targetDir, prefix, startNumber, extensionFilter);
    return success ? 0 : 1;
}