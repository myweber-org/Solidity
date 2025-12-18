
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>\n";
        return 1;
    }

    fs::path directory_path = argv[1];
    std::string base_name = argv[2];

    if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();

            std::ostringstream new_filename;
            new_filename << base_name << "_"
                         << std::setw(4) << std::setfill('0') << counter
                         << extension;

            fs::path new_path = old_path.parent_path() / new_filename.str();

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename()
                          << " -> " << new_path.filename() << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << old_path.filename()
                          << ": " << e.what() << '\n';
            }
        }
    }

    std::cout << "File renaming completed. Total files processed: "
              << (counter - 1) << '\n';
    return 0;
}