
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& prefix,
                                       int startNumber = 1,
                                       const std::string& extension = "") {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return;
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& file : files) {
            std::string newFilename = prefix + std::to_string(currentNumber);
            
            if (!extension.empty()) {
                newFilename += "." + extension;
            } else {
                newFilename += file.extension().string();
            }

            fs::path newPath = file.parent_path() / newFilename;
            
            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFilename << std::endl;
                currentNumber++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Renaming completed. Total files processed: " << currentNumber - startNumber << std::endl;
    }
};

int main() {
    std::string directory;
    std::string prefix;
    std::string extension;
    int startNumber;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);
    
    std::cout << "Enter filename prefix: ";
    std::getline(std::cin, prefix);
    
    std::cout << "Enter file extension (leave empty to keep original): ";
    std::getline(std::cin, extension);
    
    std::cout << "Enter starting number: ";
    std::cin >> startNumber;

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extension);
    
    return 0;
}