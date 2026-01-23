
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    explicit FileRenamer(const std::string& directory_path) : directory(directory_path) {}

    bool renameFiles(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cerr << "No files found in directory.\n";
            return false;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int current_number = start_number;
        bool all_renamed = true;

        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();
            std::string new_filename = prefix + std::to_string(current_number) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++current_number;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
                all_renamed = false;
            }
        }

        return all_renamed;
    }

private:
    fs::path directory;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> [prefix] [start_number]\n";
        std::cerr << "Example: " << argv[0] << " ./photos image_ 1\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = (argc >= 3) ? argv[2] : "file_";
    int start_number = (argc >= 4) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    if (renamer.renameFiles(prefix, start_number)) {
        std::cout << "Batch renaming completed successfully.\n";
    } else {
        std::cerr << "Batch renaming completed with errors.\n";
        return 1;
    }

    return 0;
}