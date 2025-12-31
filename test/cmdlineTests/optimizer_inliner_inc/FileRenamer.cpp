
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
                                       int startNumber = 1) {
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
        for (const auto& filePath : files) {
            std::string extension = filePath.extension().string();
            std::string newFileName = prefix + std::to_string(currentNumber) + extension;
            fs::path newFilePath = filePath.parent_path() / newFileName;

            try {
                fs::rename(filePath, newFilePath);
                std::cout << "Renamed: " << filePath.filename() << " -> " << newFileName << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << filePath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << (currentNumber - startNumber) << std::endl;
    }
};

int main() {
    std::string directory;
    std::string prefix;
    int startNumber;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);
    
    std::cout << "Enter filename prefix: ";
    std::getline(std::cin, prefix);
    
    std::cout << "Enter starting number (default 1): ";
    std::string input;
    std::getline(std::cin, input);
    startNumber = input.empty() ? 1 : std::stoi(input);

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber);
    
    return 0;
}