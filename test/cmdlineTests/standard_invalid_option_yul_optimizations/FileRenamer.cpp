
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix)
        : target_directory(directory), name_prefix(prefix) {}

    bool renameFiles() {
        if (!fs::exists(target_directory) || !fs::is_directory(target_directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(target_directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& old_path : files) {
            std::string extension = old_path.extension().string();
            std::string new_filename = name_prefix + "_" + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming process completed. " << (counter - 1) << " files processed." << std::endl;
        return true;
    }

private:
    std::string target_directory;
    std::string name_prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <name_prefix>" << std::endl;
        return 1;
    }

    std::string dir_path = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(dir_path, prefix);
    return renamer.renameFiles() ? 0 : 1;
}