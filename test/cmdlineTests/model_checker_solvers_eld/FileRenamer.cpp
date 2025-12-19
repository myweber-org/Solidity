
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
            fs::path newPath = fs::path(directoryPath) / newFilename;
            
            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << (counter - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name> <extension> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation .jpg 1" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    std::string extension = argv[3];
    int startNumber = (argc > 4) ? std::stoi(argv[4]) : 1;
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return 1;
    }
    
    FileRenamer::renameFilesInDirectory(directory, baseName, extension, startNumber);
    
    return 0;
}