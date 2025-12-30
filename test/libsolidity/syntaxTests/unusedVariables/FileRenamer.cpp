#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory, const std::string& baseName) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return;
    }

    std::vector<fs::directory_entry> files;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry);
        }
    }

    std::sort(files.begin(), files.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename().string() < b.path().filename().string();
              });

    int counter = 1;
    for (const auto& file : files) {
        fs::path oldPath = file.path();
        std::string extension = oldPath.extension().string();
        std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = directory / newFileName;

        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << '\n';
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
        }
    }

    std::cout << "Renaming completed. Total files processed: " << counter - 1 << '\n';
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string baseName(argv[2]);

    renameFilesSequentially(targetDir, baseName);
    return 0;
}