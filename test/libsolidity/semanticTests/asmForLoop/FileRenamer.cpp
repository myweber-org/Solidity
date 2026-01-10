
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    explicit FileRenamer(const std::string& directory) : dir_path(directory) {}

    bool renameFiles(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = start_number;
        bool all_success = true;

        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = prefix + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
                all_success = false;
            }
        }

        return all_success;
    }

private:
    fs::path dir_path;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory> [prefix] [start_number]\n";
        std::cerr << "Example: " << argv[0] << " ./photos image_ 1\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = (argc > 2) ? argv[2] : "file_";
    int start_number = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    bool success = renamer.renameFiles(prefix, start_number);

    return success ? 0 : 1;
}