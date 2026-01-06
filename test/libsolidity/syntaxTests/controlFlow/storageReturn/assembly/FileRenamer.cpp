
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    explicit FileRenamer(const std::string& directory_path, const std::string& base_name = "file")
        : directory(directory_path), prefix(base_name) {}

    bool renameFiles() {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                entries.push_back(entry);
            }
        }

        if (entries.empty()) {
            std::cout << "No regular files found in directory.\n";
            return true;
        }

        std::sort(entries.begin(), entries.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = 1;
        bool all_success = true;

        for (const auto& entry : entries) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = prefix + "_" + std::to_string(counter) + extension;
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

        std::cout << "Renaming process completed. " << (counter - 1) << " files processed.\n";
        return all_success;
    }

private:
    fs::path directory;
    std::string prefix;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path> [base_name]\n";
        std::cout << "Example: " << argv[0] << " ./photos image\n";
        return 1;
    }

    std::string dir_path = argv[1];
    std::string base_name = (argc >= 3) ? argv[2] : "file";

    FileRenamer renamer(dir_path, base_name);
    return renamer.renameFiles() ? 0 : 1;
}