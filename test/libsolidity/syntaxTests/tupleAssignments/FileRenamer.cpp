
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

            std::sort(files.begin(), files.end(),
                     [](const fs::directory_entry& a, const fs::directory_entry& b) {
                         return a.path().filename().string() < b.path().filename().string();
                     });

            int counter = startNumber;
            for (const auto& file : files) {
                fs::path oldPath = file.path();
                std::string extension = oldPath.extension().string();

                if (!targetExtension.empty() && targetExtension != extension) {
                    continue;
                }

                std::string newFilename = prefix + std::to_string(counter) + extension;
                fs::path newPath = oldPath.parent_path() / newFilename;

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                    counter++;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                }
            }

            std::cout << "Renaming completed. Total files processed: " << (counter - startNumber) << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number] [target_extension]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1 .jpg" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string targetExtension = (argc > 4) ? argv[4] : "";

    FileRenamer::renameFilesInDirectory(directoryPath, prefix, startNumber, targetExtension);
    return 0;
}