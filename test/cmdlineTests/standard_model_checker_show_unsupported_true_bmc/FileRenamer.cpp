
#include <iostream>
#include <filesystem>
#include <string>
#include <regex>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& pattern, const std::string& replacement) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return;
    }

    std::regex regexPattern(pattern);
    int count = 0;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string oldName = entry.path().filename().string();
                std::string newName = std::regex_replace(oldName, regexPattern, replacement);

                if (oldName != newName && !newName.empty()) {
                    fs::path oldPath = entry.path();
                    fs::path newPath = entry.path().parent_path() / newName;

                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldName << " -> " << newName << '\n';
                    ++count;
                }
            }
        }
        std::cout << "Total files renamed: " << count << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error during renaming: " << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <regex_pattern> <replacement>\n";
        std::cout << "Example: " << argv[0] << " ./files \"(.*)\\.txt\" \"$1_backup.txt\"\n";
        return 1;
    }

    fs::path dir(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(dir, pattern, replacement);

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
    static void renameFilesInDirectory(const std::string& directoryPath,
                                       const std::string& baseName,
                                       const std::string& extension,
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

        int counter = startNumber;
        for (const auto& oldPath : files) {
            std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = fs::path(directoryPath) / newFileName;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. " << (counter - startNumber) << " files processed." << std::endl;
    }
};

int main() {
    std::string directory = "./test_files";
    std::string baseName = "document";
    std::string extension = ".txt";

    FileRenamer::renameFilesInDirectory(directory, baseName, extension);

    return 0;
}