
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
                                       int startNumber = 1,
                                       const std::string& extension = "") {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        int counter = startNumber;
        for (const auto& file : files) {
            std::string newFileName = baseName + "_" + std::to_string(counter);
            
            if (!extension.empty()) {
                newFileName += "." + extension;
            } else {
                newFileName += file.extension().string();
            }
            
            fs::path newPath = file.parent_path() / newFileName;
            
            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFileName << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Renaming complete. " << (counter - startNumber) << " files processed." << std::endl;
    }
    
    static void previewRenames(const std::string& directoryPath,
                               const std::string& baseName,
                               int startNumber = 1,
                               const std::string& extension = "") {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        std::cout << "Preview of renames:" << std::endl;
        std::cout << "-------------------" << std::endl;
        
        int counter = startNumber;
        for (const auto& file : files) {
            std::string newFileName = baseName + "_" + std::to_string(counter);
            
            if (!extension.empty()) {
                newFileName += "." + extension;
            } else {
                newFileName += file.extension().string();
            }
            
            std::cout << file.filename() << " -> " << newFileName << std::endl;
            counter++;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name> [start_number] [extension]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation 1 jpg" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extension = (argc > 4) ? argv[4] : "";
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return 1;
    }
    
    FileRenamer::previewRenames(directory, baseName, startNumber, extension);
    
    std::cout << "\nProceed with rename? (y/n): ";
    char response;
    std::cin >> response;
    
    if (response == 'y' || response == 'Y') {
        FileRenamer::renameFilesInDirectory(directory, baseName, startNumber, extension);
    } else {
        std::cout << "Operation cancelled." << std::endl;
    }
    
    return 0;
}