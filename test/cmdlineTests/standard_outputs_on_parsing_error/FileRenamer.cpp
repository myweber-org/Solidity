#include <iostream>
#include <filesystem>
#include <string>
#include <regex>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& pattern, const std::string& replacement) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return;
    }

    std::regex regexPattern(pattern);
    std::vector<fs::directory_entry> files;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            files.push_back(entry);
        }
    }

    std::sort(files.begin(), files.end());

    for (const auto& file : files) {
        std::string oldName = file.path().filename().string();
        std::string newName = std::regex_replace(oldName, regexPattern, replacement);

        if (oldName != newName && !newName.empty()) {
            fs::path newPath = file.path().parent_path() / newName;
            try {
                fs::rename(file.path(), newPath);
                std::cout << "Renamed: " << oldName << " -> " << newName << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldName << ": " << e.what() << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <regex_pattern> <replacement>\n";
        std::cout << "Example: " << argv[0] << " ./files \"(.*)\\.txt\" \"$1_backup.txt\"\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(targetDir, pattern, replacement);
    return 0;
}