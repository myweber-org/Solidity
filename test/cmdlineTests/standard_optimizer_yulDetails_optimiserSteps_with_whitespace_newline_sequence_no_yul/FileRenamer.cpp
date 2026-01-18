
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

        int counter = startNumber;
        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() 
                          << " -> " << newPath.filename() << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() 
                          << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming complete. Processed " << (counter - startNumber) 
                  << " files." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] 
                  << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] 
                  << " ./photos vacation_ 1" << std::endl;
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
    static void renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       const std::string& extension) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int counter = 1;
        for (const auto& file : files) {
            std::string newName = prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = directory / newName;

            try {
                fs::rename(file.path(), newPath);
                std::cout << "Renamed: " << file.path().filename() << " -> " << newName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.path().filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. " << (counter - 1) << " files processed." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation .jpg" << std::endl;
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string prefix(argv[2]);
    std::string extension(argv[3]);

    FileRenamer::renameFilesInDirectory(dirPath, prefix, extension);

    return 0;
}