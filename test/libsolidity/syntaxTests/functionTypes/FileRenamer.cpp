
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
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = start_number;
        for (const auto& old_path : files) {
            std::string extension = old_path.extension().string();
            std::string new_filename = prefix + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << (counter - start_number) << " files." << std::endl;
        return true;
    }

private:
    fs::path dir_path;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [start_number]" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int start_number = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    return renamer.renameFiles(prefix, start_number) ? 0 : 1;
}