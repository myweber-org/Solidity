
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
                                       const std::string& extension,
                                       int startNumber = 1) {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::string newFilename = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;
            
            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << (counter - startNumber) << std::endl;
    }
};

int main() {
    std::string directory;
    std::string baseName;
    std::string extension;
    
    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);
    
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, baseName);
    
    std::cout << "Enter file extension (including dot): ";
    std::getline(std::cin, extension);
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Invalid directory path!" << std::endl;
        return 1;
    }
    
    FileRenamer::renameFilesInDirectory(directory, baseName, extension);
    
    return 0;
}