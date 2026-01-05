
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
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
    
    static void previewRenaming(const std::string& directoryPath,
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
        
        std::cout << "Preview of renaming operations:" << std::endl;
        int counter = startNumber;
        for (const auto& file : files) {
            std::string newFilename = baseName + "_" + std::to_string(counter) + extension;
            std::cout << file.filename() << " -> " << newFilename << std::endl;
            counter++;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name> <extension> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos image .jpg 1" << std::endl;
        std::cout << "For preview only, add --preview flag" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];
    int startNumber = (argc > 4) ? std::stoi(argv[4]) : 1;
    
    bool previewMode = false;
    if (argc > 5 && std::string(argv[5]) == "--preview") {
        previewMode = true;
    }
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
        return 1;
    }
    
    if (previewMode) {
        FileRenamer::previewRenaming(directory, baseName, extension, startNumber);
    } else {
        std::cout << "Proceed with renaming? (y/n): ";
        char response;
        std::cin >> response;
        
        if (response == 'y' || response == 'Y') {
            FileRenamer::renameFilesInDirectory(directory, baseName, extension, startNumber);
        } else {
            std::cout << "Operation cancelled." << std::endl;
        }
    }
    
    return 0;
}