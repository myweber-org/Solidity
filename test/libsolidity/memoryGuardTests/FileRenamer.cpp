
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    explicit FileRenamer(const std::string& directory_path) : directory(directory_path) {}

    bool rename_files(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
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
        bool all_success = true;

        for (const auto& old_path : files) {
            std::string extension = old_path.extension().string();
            std::string new_filename = prefix + std::to_string(counter) + extension;
            fs::path new_path = directory / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
                all_success = false;
            }
        }

        return all_success;
    }

private:
    fs::path directory;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos image_" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int start_number = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    bool success = renamer.rename_files(prefix, start_number);

    return success ? 0 : 1;
}