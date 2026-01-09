
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
        
        std::cout << "Total files renamed: " << (counter - 1) << std::endl;
    }
    
    static bool validateExtension(const std::string& ext) {
        if (ext.empty() || ext[0] != '.') {
            std::cerr << "Extension must start with '.' (e.g., '.txt', '.jpg')" << std::endl;
            return false;
        }
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation .jpg" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];
    
    if (!FileRenamer::validateExtension(extension)) {
        return 1;
    }
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Directory '" << directory << "' does not exist or is not accessible." << std::endl;
        return 1;
    }
    
    std::cout << "Renaming files in: " << directory << std::endl;
    std::cout << "Base name: " << baseName << std::endl;
    std::cout << "Extension: " << extension << std::endl;
    std::cout << "Proceed? (y/n): ";
    
    char response;
    std::cin >> response;
    
    if (response == 'y' || response == 'Y') {
        FileRenamer::renameFilesInDirectory(directory, baseName, extension);
    } else {
        std::cout << "Operation cancelled." << std::endl;
    }
    
    return 0;
}