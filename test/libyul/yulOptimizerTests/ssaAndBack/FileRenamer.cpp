
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
                                       int startNumber = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
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

        std::cout << "Renaming completed. Total files processed: " << (counter - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer::renameFilesInDirectory(targetDir, prefix, startNumber);
    
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
    explicit FileRenamer(const std::string& directory_path) : directory(directory_path) {}

    bool rename_files(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                entries.push_back(entry);
            }
        }

        if (entries.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(entries.begin(), entries.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int current_number = start_number;
        for (const auto& entry : entries) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = prefix + std::to_string(current_number) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++current_number;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Renaming completed successfully." << std::endl;
        return true;
    }

private:
    fs::path directory;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        return 1;
    }

    std::string directory_path = argv[1];
    std::string prefix = argv[2];
    int start_number = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory_path);
    if (!renamer.rename_files(prefix, start_number)) {
        return 1;
    }

    return 0;
}