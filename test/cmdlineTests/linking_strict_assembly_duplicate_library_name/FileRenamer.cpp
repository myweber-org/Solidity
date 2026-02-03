
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesSequentially(const std::string& directoryPath, const std::string& baseName) {
    try {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path.\n";
            return;
        }

        int counter = 1;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.path())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();

                std::stringstream newFilename;
                newFilename << baseName << "_" << std::setw(4) << std::setfill('0') << counter << extension;
                fs::path newPath = oldPath.parent_path() / newFilename.str();

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << "\n";
                    counter++;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
                }
            }
        }
        std::cout << "Renaming complete. Total files processed: " << (counter - 1) << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
}

int main() {
    std::string dirPath;
    std::string baseName;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, dirPath);
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);

    renameFilesSequentially(dirPath, baseName);
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
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& baseName,
                                       const std::string& extension) {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        int counter = 1;
        for (const auto& file : files) {
            std::string newName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = fs::path(directoryPath) / newName;
            
            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newName << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << counter - 1 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos image .jpg" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return 1;
    }
    
    FileRenamer::renameFilesInDirectory(directory, baseName, extension);
    
    return 0;
}