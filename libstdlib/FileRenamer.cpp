
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesSequentially(const std::string& directoryPath, const std::string& baseName) {
    int counter = 1;
    
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
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
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }
    
    std::string directoryPath = argv[1];
    std::string baseName = argv[2];
    
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }
    
    renameFilesSequentially(directoryPath, baseName);
    
    return 0;
}