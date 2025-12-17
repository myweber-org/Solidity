
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const std::string& directoryPath, const std::string& prefix) {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        int counter = 1;
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newFilename = prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = file.parent_path() / newFilename;
            
            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFilename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << counter - 1 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        return 1;
    }
    
    std::string directoryPath = argv[1];
    std::string prefix = argv[2];
    
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        std::cerr << "Error: Invalid directory path" << std::endl;
        return 1;
    }
    
    FileRenamer::renameFilesInDirectory(directoryPath, prefix);
    
    return 0;
}