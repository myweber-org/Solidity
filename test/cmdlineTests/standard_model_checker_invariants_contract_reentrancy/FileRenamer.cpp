#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path directory_path = argv[1];

    if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
        std::cerr << "Error: Provided path is not a valid directory." << std::endl;
        return 1;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();

            std::ostringstream new_filename;
            new_filename << "file_" << std::setw(4) << std::setfill('0') << counter << extension;
            fs::path new_path = old_path.parent_path() / new_filename.str();

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_path.filename() << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << old_path.filename() << ": " << e.what() << std::endl;
            }
        }
    }

    std::cout << "Renaming complete. Total files processed: " << (counter - 1) << std::endl;
    return 0;
}