#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>

namespace fs = std::filesystem;

bool compareByModTime(const fs::directory_entry& a, const fs::directory_entry& b) {
    return fs::last_write_time(a) < fs::last_write_time(b);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    std::vector<fs::directory_entry> files;
    for (const auto& entry : fs::directory_iterator(targetDir)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry);
        }
    }

    std::sort(files.begin(), files.end(), compareByModTime);

    int counter = 1;
    for (const auto& file : files) {
        fs::path oldPath = file.path();
        std::string extension = oldPath.extension().string();
        std::string newName = "file_" + std::to_string(counter) + extension;
        fs::path newPath = oldPath.parent_path() / newName;

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newName << '\n';
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
        }
        ++counter;
    }

    std::cout << "Renaming complete.\n";
    return 0;
}