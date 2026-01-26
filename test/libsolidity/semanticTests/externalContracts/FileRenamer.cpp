
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
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                if (targetExtension.empty() ||
                    entry.path().extension().string() == targetExtension) {
                    files.push_back(entry.path());
                }
            }
        }

        std::sort(files.begin(), files.end());

        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(counter) + oldPath.extension().string();
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << (counter - startNumber) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [startNumber] [extension]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1 .jpg" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;
    std::string extension = (argc > 4) ? argv[4] : "";

    FileRenamer::renameFilesInDirectory(directory, prefix, startNumber, extension);

    return 0;
}#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>\n";
        return 1;
    }

    fs::path dir_path(argv[1]);

    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (fs::is_regular_file(entry.status())) {
            entries.push_back(entry);
        }
    }

    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename().string() < b.path().filename().string();
              });

    int counter = 1;
    for (const auto& entry : entries) {
        fs::path old_path = entry.path();
        std::string extension = old_path.extension().string();

        std::ostringstream new_filename;
        new_filename << std::setw(4) << std::setfill('0') << counter << extension;
        fs::path new_path = old_path.parent_path() / new_filename.str();

        try {
            fs::rename(old_path, new_path);
            std::cout << "Renamed: " << old_path.filename() << " -> " << new_path.filename() << '\n';
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << old_path.filename() << ": " << e.what() << '\n';
        }
    }

    std::cout << "File renaming completed.\n";
    return 0;
}