
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory, const std::string& prefix, int startNumber) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int currentNumber = startNumber;
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newFilename << std::endl;
                ++currentNumber;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.filename() << ": " << e.what() << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> <start_number>" << std::endl;
        return 1;
    }

    fs::path directory(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = std::stoi(argv[3]);

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber);
    return 0;
}