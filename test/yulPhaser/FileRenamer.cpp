
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    explicit FileRenamer(const std::string& directory_path) : dir_path(directory_path) {}

    bool rename_files(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
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

        int current_number = start_number;
        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = prefix + std::to_string(current_number) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++current_number;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << files.size() << " files." << std::endl;
        return true;
    }

private:
    std::string dir_path;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> [prefix] [start_number]" << std::endl;
        std::cerr << "Example: " << argv[0] << " ./photos image_" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = (argc >= 3) ? argv[2] : "file_";
    int start_number = (argc >= 4) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    if (!renamer.rename_files(prefix, start_number)) {
        return 1;
    }

    return 0;
}