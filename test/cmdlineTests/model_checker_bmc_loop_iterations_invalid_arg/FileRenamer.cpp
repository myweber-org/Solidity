
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void batchRename(const std::string& directory, const std::string& baseName) {
    int counter = 1;
    
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                fs::path oldPath = entry.path();
                std::string extension = oldPath.extension().string();
                
                std::stringstream newFilename;
                newFilename << baseName << "_" 
                           << std::setw(4) << std::setfill('0') << counter 
                           << extension;
                
                fs::path newPath = oldPath.parent_path() / newFilename.str();
                
                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() 
                              << " -> " << newPath.filename() << std::endl;
                    counter++;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Error renaming " << oldPath.filename() 
                              << ": " << e.what() << std::endl;
                }
            }
        }
        
        std::cout << "Total files renamed: " << (counter - 1) << std::endl;
        
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Directory error: " << e.what() << std::endl;
    }
}

int main() {
    std::string targetDirectory;
    std::string namingPattern;
    
    std::cout << "Enter directory path: ";
    std::getline(std::cin, targetDirectory);
    
    std::cout << "Enter base name for files: ";
    std::getline(std::cin, namingPattern);
    
    if (fs::exists(targetDirectory) && fs::is_directory(targetDirectory)) {
        batchRename(targetDirectory, namingPattern);
    } else {
        std::cerr << "Invalid directory path." << std::endl;
        return 1;
    }
    
    return 0;
}