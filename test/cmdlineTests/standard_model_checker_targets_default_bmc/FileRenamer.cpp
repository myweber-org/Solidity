
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFiles(const std::string& directory, const std::string& prefix, const std::string& extension) {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        int counter = 1;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = fs::path(directory) / newFilename;
            
            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << counter - 1 << std::endl;
    }
};

int main() {
    std::string directory = "./documents";
    std::string prefix = "document";
    std::string extension = ".txt";
    
    FileRenamer::renameFiles(directory, prefix, extension);
    
    return 0;
}