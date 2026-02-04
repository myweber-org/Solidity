
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& baseName) {
    std::vector<fs::directory_entry> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
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
        
        std::string newFilename = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = directory / newFilename;
        
        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Renaming complete. Processed " << (counter - 1) << " files." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }
    
    fs::path targetDirectory(argv[1]);
    std::string baseName(argv[2]);
    
    if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }
    
    renameFilesInDirectory(targetDirectory, baseName);
    
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
    explicit FileRenamer(const std::string& directory) : dir_path(directory) {}

    bool renameFiles(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int current_number = start_number;
        for (const auto& file : files) {
            fs::path old_path = file.path();
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

        std::cout << "Successfully renamed " << files.size() << " file(s)." << std::endl;
        return true;
    }

private:
    fs::path dir_path;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory> [prefix] [start_number]" << std::endl;
        std::cerr << "Example: " << argv[0] << " ./photos image_" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = (argc >= 3) ? argv[2] : "file_";
    int start_number = (argc >= 4) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    if (!renamer.renameFiles(prefix, start_number)) {
        return 1;
    }

    return 0;
}