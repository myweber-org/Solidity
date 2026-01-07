
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    explicit FileRenamer(const std::string& directory) : dir_path(directory) {}

    bool renameFiles(const std::string& baseName, const std::string& extension) {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& old_path : files) {
            std::string new_filename = baseName + "_" + std::to_string(counter) + "." + extension;
            fs::path new_path = dir_path / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << "\n";
                return false;
            }
        }

        std::cout << "Successfully renamed " << (counter - 1) << " files.\n";
        return true;
    }

private:
    fs::path dir_path;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory> <base_name> <extension>\n";
        std::cerr << "Example: " << argv[0] << " ./photos image png\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];

    FileRenamer renamer(directory);
    if (!renamer.renameFiles(baseName, extension)) {
        return 1;
    }

    return 0;
}