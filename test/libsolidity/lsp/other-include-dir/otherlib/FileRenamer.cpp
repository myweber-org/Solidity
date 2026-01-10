
#include <iostream>
#include <filesystem>
#include <string>
#include <regex>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& pattern, const std::string& replacement) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Provided path is not a valid directory." << std::endl;
        return;
    }

    std::regex regexPattern(pattern);
    int renamedCount = 0;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string oldName = entry.path().filename().string();
            std::string newName = std::regex_replace(oldName, regexPattern, replacement);

            if (oldName != newName && !newName.empty()) {
                fs::path oldPath = entry.path();
                fs::path newPath = entry.path().parent_path() / newName;

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldName << " -> " << newName << std::endl;
                    ++renamedCount;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldName << ": " << e.what() << std::endl;
                }
            }
        }
    }

    std::cout << "Total files renamed: " << renamedCount << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <regex_pattern> <replacement_string>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(targetDir, pattern, replacement);
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
                                       const std::string& extension) {
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

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << "\n";
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
            }
        }

        std::cout << "Renaming completed. Total files processed: " << (counter - 1) << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> <extension>\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation .jpg\n";
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string prefix(argv[2]);
    std::string extension(argv[3]);

    FileRenamer::renameFilesInDirectory(dirPath, prefix, extension);

    return 0;
}