
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       const std::string& extension,
                                       int startNumber = 1,
                                       int digits = 4) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::stringstream newFilename;
            newFilename << prefix
                        << std::setw(digits) << std::setfill('0') << counter
                        << extension;

            fs::path newPath = directory / newFilename.str();

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << '\n';
            }
        }

        std::cout << "Renaming complete. Total files processed: " << (counter - startNumber) << '\n';
    }
};

int main() {
    std::string dirPath;
    std::cout << "Enter directory path: ";
    std::getline(std::cin, dirPath);

    std::string prefix;
    std::cout << "Enter filename prefix: ";
    std::getline(std::cin, prefix);

    std::string extension;
    std::cout << "Enter file extension (including dot, e.g., .txt): ";
    std::getline(std::cin, extension);

    int startNum;
    std::cout << "Enter starting number: ";
    std::cin >> startNum;

    int digitCount;
    std::cout << "Enter number of digits for padding: ";
    std::cin >> digitCount;

    FileRenamer::renameFilesInDirectory(dirPath, prefix, extension, startNum, digitCount);

    return 0;
}
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& baseName) {
    std::vector<fs::directory_entry> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            files.push_back(entry);
        }
    }
    
    std::sort(files.begin(), files.end(), 
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename().string() < b.path().filename().string();
              });
    
    int counter = 1;
    for (const auto& file : files) {
        fs::path oldPath = file.path();
        std::string extension = oldPath.extension().string();
        
        std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = directory / newFileName;
        
        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Total files renamed: " << (counter - 1) << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }
    
    fs::path targetDirectory(argv[1]);
    std::string baseName(argv[2]);
    
    if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }
    
    renameFilesInDirectory(targetDirectory, baseName);
    
    return 0;
}