#include <iostream>
#include <filesystem>
#include <string>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& pattern, const std::string& replacement) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return;
    }

    std::regex regexPattern(pattern);
    std::vector<std::pair<fs::path, fs::path>> renameOperations;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string oldName = entry.path().filename().string();
            std::string newName = std::regex_replace(oldName, regexPattern, replacement);

            if (oldName != newName && !newName.empty()) {
                fs::path newPath = entry.path().parent_path() / newName;
                renameOperations.emplace_back(entry.path(), newPath);
            }
        }
    }

    if (renameOperations.empty()) {
        std::cout << "No files matched the pattern for renaming.\n";
        return;
    }

    std::cout << "The following files will be renamed:\n";
    for (const auto& op : renameOperations) {
        std::cout << "  " << op.first.filename() << " -> " << op.second.filename() << '\n';
    }

    std::cout << "Proceed? (y/n): ";
    char confirm;
    std::cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        for (const auto& op : renameOperations) {
            try {
                fs::rename(op.first, op.second);
                std::cout << "Renamed: " << op.first.filename() << " -> " << op.second.filename() << '\n';
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << op.first.filename() << ": " << e.what() << '\n';
            }
        }
        std::cout << "Renaming complete.\n";
    } else {
        std::cout << "Renaming cancelled.\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory> <regex_pattern> <replacement_string>\n";
        std::cerr << "Example: " << argv[0] << " ./files \"(.*)\\.txt\" \"$1_backup.txt\"\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(targetDir, pattern, replacement);

    return 0;
}