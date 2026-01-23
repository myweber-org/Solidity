
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
                                       const std::string& targetExtension = "") {
        try {
            if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
                std::cerr << "Error: Invalid directory path." << std::endl;
                return;
            }

            std::vector<fs::directory_entry> files;
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                if (fs::is_regular_file(entry.status())) {
                    files.push_back(entry);
                }
            }

            if (files.empty()) {
                std::cout << "No files found in directory." << std::endl;
                return;
            }

            std::sort(files.begin(), files.end(),
                     [](const fs::directory_entry& a, const fs::directory_entry& b) {
                         return a.path().filename().string() < b.path().filename().string();
                     });

            int currentNumber = startNumber;
            for (const auto& file : files) {
                fs::path oldPath = file.path();
                std::string extension = oldPath.extension().string();

                if (!targetExtension.empty() && targetExtension != extension) {
                    continue;
                }

                std::string newFilename = prefix + std::to_string(currentNumber) + extension;
                fs::path newPath = oldPath.parent_path() / newFilename;

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                    currentNumber++;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                }
            }

            std::cout << "Renaming completed. Total files processed: " << (currentNumber - startNumber) << std::endl;

        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::string directory;
    std::string prefix;
    std::string extensionFilter;
    int startNumber;

    std::cout << "Enter directory path: ";
    std::getline(std::cin, directory);

    std::cout << "Enter filename prefix: ";
    std::getline(std::cin, prefix);

    std::cout << "Enter starting number (default 1): ";
    std::string startInput;
    std::getline(std::cin, startInput);
    startNumber = startInput.empty() ? 1 : std::stoi(startInput);

    std::cout << "Enter file extension to filter (e.g., .txt, leave empty for all): ";
    std::getline(std::cin, extensionFilter);

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extensionFilter);

    return 0;
}
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
            std::cerr << "Error: Directory does not exist or is not accessible." << std::endl;
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
            
            std::string newFilename = prefix + "_" + std::to_string(currentNumber) + extension;
            fs::path newPath = directory / newFilename;
            
            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                currentNumber++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Renaming completed. Total files processed: " << (currentNumber - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation 1" << std::endl;
        return 1;
    }
    
    fs::path directory(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc >= 4) ? std::stoi(argv[3]) : 1;
    
    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber);
    
    return 0;
}