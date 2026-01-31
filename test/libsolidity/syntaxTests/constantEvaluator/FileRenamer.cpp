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

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.path().filename().string() < b.path().filename().string();
            });

        int counter = 1;
        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();
            std::string new_filename = name_prefix + "_" + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << '\n';
        return true;
    }

private:
    std::string dir_path;
    std::string name_prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <name_prefix>\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(directory, prefix);
    if (!renamer.renameFiles()) {
        return 1;
    }

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
    static bool renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& prefix,
                                       int startNumber = 1,
                                       const std::string& extensionFilter = "") {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                if (extensionFilter.empty() ||
                    entry.path().extension() == extensionFilter) {
                    files.push_back(entry.path());
                }
            }
        }

        if (files.empty()) {
            std::cout << "No files found matching criteria." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(currentNumber) + oldPath.extension().string();
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                currentNumber++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << files.size() << " files." << std::endl;
        return true;
    }
};

int main() {
    std::string directory = "./test_files";
    std::string prefix = "document_";
    
    bool success = FileRenamer::renameFilesInDirectory(directory, prefix, 1, ".txt");
    
    if (success) {
        std::cout << "Operation completed successfully." << std::endl;
        return 0;
    } else {
        std::cout << "Operation failed." << std::endl;
        return 1;
    }
}