#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& baseName)
        : dirPath(directory), newBaseName(baseName) {}

    bool renameFiles() {
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cerr << "No regular files found in the directory." << std::endl;
            return false;
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
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming process completed." << std::endl;
        return true;
    }

private:
    std::string dirPath;
    std::string newBaseName;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <new_base_name>" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string baseName = argv[2];

    FileRenamer renamer(directory, baseName);
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

void batchRename(const std::string& directory, const std::string& baseName) {
    std::vector<fs::path> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry.path());
        }
    }
    
    std::sort(files.begin(), files.end());
    
    int counter = 1;
    for (const auto& file : files) {
        std::string extension = file.extension().string();
        std::string newName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = fs::path(directory) / newName;
        
        try {
            fs::rename(file, newPath);
            std::cout << "Renamed: " << file.filename() << " -> " << newName << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Renaming complete. " << (counter - 1) << " files processed." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return 1;
    }
    
    batchRename(directory, baseName);
    return 0;
}