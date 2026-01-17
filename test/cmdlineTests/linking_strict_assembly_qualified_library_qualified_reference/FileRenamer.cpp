
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
            std::string newFileName = baseName + "_" + 
                                     std::to_string(counter) + 
                                     extension;
            fs::path newPath = fs::path(directoryPath) / newFileName;
            
            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() 
                         << " -> " << newFileName << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() 
                         << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << (counter - 1) << std::endl;
    }
    
    static bool validateDirectory(const std::string& path) {
        return fs::exists(path) && fs::is_directory(path);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] 
                 << " <directory> <base_name> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] 
                 << " ./photos vacation .jpg" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];
    
    if (!FileRenamer::validateDirectory(directory)) {
        std::cerr << "Error: Invalid directory path: " << directory << std::endl;
        return 1;
    }
    
    if (extension.empty() || extension[0] != '.') {
        std::cerr << "Error: Extension must start with '.' (e.g., .txt, .jpg)" << std::endl;
        return 1;
    }
    
    std::cout << "Starting batch rename operation..." << std::endl;
    std::cout << "Directory: " << directory << std::endl;
    std::cout << "Base name: " << baseName << std::endl;
    std::cout << "Extension: " << extension << std::endl;
    
    char confirm;
    std::cout << "Proceed with rename? (y/n): ";
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        FileRenamer::renameFilesInDirectory(directory, baseName, extension);
    } else {
        std::cout << "Operation cancelled." << std::endl;
    }
    
    return 0;
}