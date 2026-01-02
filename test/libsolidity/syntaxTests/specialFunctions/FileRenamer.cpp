
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& baseName)
        : targetDirectory(directory), newBaseName(baseName) {}

    bool renameFiles() {
        if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(targetDirectory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No regular files found in directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = 1;
        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();
            std::string newFileName = newBaseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
            }
        }

        std::cout << "Renaming completed. " << (counter - 1) << " files processed.\n";
        return true;
    }

private:
    std::string targetDirectory;
    std::string newBaseName;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <new_base_name>\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string baseName = argv[2];

    FileRenamer renamer(directory, baseName);
    return renamer.renameFiles() ? 0 : 1;
}