
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory_path, const std::string& prefix = "file_")
        : dir_path(directory_path), name_prefix(prefix) {}

    bool renameFiles() {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Invalid directory path.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No regular files found in the directory.\n";
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

            std::string new_filename = name_prefix + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
            }
        }

        std::cout << "Renaming process completed.\n";
        return true;
    }

private:
    std::string dir_path;
    std::string name_prefix;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> [prefix]\n";
        std::cerr << "Example: " << argv[0] << " ./my_folder doc_\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = (argc >= 3) ? argv[2] : "file_";

    FileRenamer renamer(directory, prefix);
    return renamer.renameFiles() ? 0 : 1;
}