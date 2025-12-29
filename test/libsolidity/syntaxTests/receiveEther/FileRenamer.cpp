
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory) {
    std::vector<fs::directory_entry> files;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
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
        std::string extension = oldPath.extension().string();

        std::ostringstream newFilename;
        newFilename << "file_" << std::setw(4) << std::setfill('0') << counter << extension;
        fs::path newPath = directory / newFilename.str();

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << std::endl;
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }

    std::cout << "Total files processed: " << counter - 1 << std::endl;
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