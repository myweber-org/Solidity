
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
            std::cerr << "Error: Invalid directory path.\n";
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
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
                std::cout << "Renamed: " << oldPath.filename()
                          << " -> " << newPath.filename() << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename()
                          << ": " << e.what() << '\n';
            }
        }

        std::cout << "Renaming completed. Total files processed: "
                  << (counter - startNumber) << '\n';
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
                  << " <directory> <prefix> <extension> [start] [digits]\n"
                  << "Example: " << argv[0]
                  << " ./images photo_ .jpg 1 3\n";
        return 1;
    }

    fs::path dir(argv[1]);
    std::string prefix(argv[2]);
    std::string extension(argv[3]);
    int start = (argc > 4) ? std::stoi(argv[4]) : 1;
    int digits = (argc > 5) ? std::stoi(argv[5]) : 4;

    FileRenamer::renameFilesInDirectory(dir, prefix, extension, start, digits);

    return 0;
}