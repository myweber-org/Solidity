
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory) {
    std::vector<fs::directory_entry> files;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry);
        }
    }

    std::sort(files.begin(), files.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return fs::last_write_time(a) < fs::last_write_time(b);
              });

    int counter = 1;
    for (const auto& file : files) {
        fs::path oldPath = file.path();
        fs::path extension = oldPath.extension();
        fs::path newPath = directory / (std::to_string(counter) + extension.string());

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);

    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        std::cerr << "Invalid directory: " << targetDir << std::endl;
        return 1;
    }

    renameFilesSequentially(targetDir);
    return 0;
}