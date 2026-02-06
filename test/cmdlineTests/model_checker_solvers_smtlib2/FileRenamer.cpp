#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       const std::string& extension) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory.\n";
            return;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = 1;
        for (const auto& file : files) {
            std::string newFilename = prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(file.path(), newPath);
                std::cout << "Renamed: " << file.path().filename() << " -> " << newFilename << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.path().filename() << ": " << e.what() << "\n";
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> <extension>\n";
        std::cout << "Example: " << argv[0] << " ./images vacation .jpg\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);
    std::string extension(argv[3]);

    if (extension[0] != '.') {
        extension = "." + extension;
    }

    FileRenamer::renameFilesInDirectory(targetDir, prefix, extension);
    return 0;
}