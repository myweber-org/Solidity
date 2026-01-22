
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static bool renameFilesInDirectory(const fs::path& directory, 
                                       const std::string& prefix, 
                                       int startNumber = 1) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end(), 
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int currentNumber = startNumber;
        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();
            
            std::string newFilename = prefix + std::to_string(currentNumber) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() 
                          << " -> " << newFilename << std::endl;
                currentNumber++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() 
                          << ": " << e.what() << std::endl;
                return false;
            }
        }

        std::cout << "Successfully renamed " << files.size() << " files." << std::endl;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [start_number]" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1" << std::endl;
        return 1;
    }

    fs::path directory(argv[1]);
    std::string prefix(argv[2]);
    int startNumber = (argc >= 4) ? std::stoi(argv[3]) : 1;

    return FileRenamer::renameFilesInDirectory(directory, prefix, startNumber) ? 0 : 1;
}#include <iostream>
#include <filesystem>
#include <string>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static bool renameFilesInDirectory(const fs::path& directory,
                                       const std::string& searchPattern,
                                       const std::string& replacePattern) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::regex patternRegex(searchPattern);
        std::vector<std::pair<fs::path, fs::path>> renameOperations;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::string newFilename = std::regex_replace(filename, patternRegex, replacePattern);

                if (filename != newFilename) {
                    fs::path newPath = entry.path().parent_path() / newFilename;
                    renameOperations.emplace_back(entry.path(), newPath);
                }
            }
        }

        if (renameOperations.empty()) {
            std::cout << "No files matched the rename pattern." << std::endl;
            return true;
        }

        std::cout << "The following files will be renamed:" << std::endl;
        for (const auto& op : renameOperations) {
            std::cout << "  " << op.first.filename() << " -> " << op.second.filename() << std::endl;
        }

        std::cout << "Proceed with rename? (y/n): ";
        char confirm;
        std::cin >> confirm;

        if (confirm == 'y' || confirm == 'Y') {
            for (const auto& op : renameOperations) {
                try {
                    fs::rename(op.first, op.second);
                    std::cout << "Renamed: " << op.first.filename() << std::endl;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << op.first.filename()
                              << ": " << e.what() << std::endl;
                }
            }
            std::cout << "Rename operation completed." << std::endl;
            return true;
        } else {
            std::cout << "Rename operation cancelled." << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <directory> <search_regex> <replace_pattern>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string searchPattern(argv[2]);
    std::string replacePattern(argv[3]);

    return FileRenamer::renameFilesInDirectory(targetDir, searchPattern, replacePattern) ? 0 : 1;
}