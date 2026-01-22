
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory, const std::string& prefix) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);

    FileRenamer::renameFilesInDirectory(targetDir, prefix);
    return 0;
}
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix)
        : dir_path(directory), name_prefix(prefix) {}

    bool renameFiles() {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& old_path : files) {
            std::string extension = old_path.extension().string();
            std::string new_filename = name_prefix + "_" + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << "\n";
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << "\n";
        return true;
    }

private:
    std::string dir_path;
    std::string name_prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <name_prefix>\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(directory, prefix);
    return renamer.renameFiles() ? 0 : 1;
}