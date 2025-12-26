
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory, 
                                       const std::string& prefix, 
                                       int startNumber = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        std::sort(files.begin(), files.end(), 
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int currentNumber = startNumber;
        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();
            
            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = directory / newFilename;
            
            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() 
                          << " -> " << newPath.filename() << std::endl;
                currentNumber++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() 
                          << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << (currentNumber - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1" << std::endl;
        return 1;
    }
    
    fs::path directory(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc >= 4) ? std::stoi(argv[3]) : 1;
    
    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber);
    
    return 0;
}