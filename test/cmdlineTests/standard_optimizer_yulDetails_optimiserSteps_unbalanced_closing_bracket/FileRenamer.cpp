
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void batchRename(const std::string& directory, const std::string& extension) {
    std::vector<fs::path> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            if (extension.empty() || entry.path().extension() == extension) {
                files.push_back(entry.path());
            }
        }
    }
    
    std::sort(files.begin(), files.end());
    
    int counter = 1;
    for (const auto& file : files) {
        std::string newName = directory + "/" + std::to_string(counter) + file.extension().string();
        fs::path newPath(newName);
        
        try {
            fs::rename(file, newPath);
            std::cout << "Renamed: " << file.filename() << " -> " << newPath.filename() << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <directory> [extension]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos .jpg" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string extension = (argc > 2) ? argv[2] : "";
    
    if (!extension.empty() && extension[0] != '.') {
        extension = "." + extension;
    }
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return 1;
    }
    
    batchRename(directory, extension);
    
    return 0;
}