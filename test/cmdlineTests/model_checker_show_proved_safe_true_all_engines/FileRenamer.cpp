
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