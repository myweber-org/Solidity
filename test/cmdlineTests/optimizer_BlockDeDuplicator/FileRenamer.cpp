
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>\n";
        return 1;
    }

    fs::path directory_path = argv[1];
    if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();
            std::string new_filename = "file_" + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << old_path.filename() << ": " << e.what() << '\n';
            }
        }
    }

    std::cout << "Renaming process completed.\n";
    return 0;
}