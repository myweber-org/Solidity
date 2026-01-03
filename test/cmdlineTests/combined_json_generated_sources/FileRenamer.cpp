#include <iostream>
#include <filesystem>
#include <string>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& pattern, const std::string& replacement) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Directory does not exist or is not accessible.\n";
        return;
    }

    std::regex regexPattern(pattern);
    std::vector<fs::path> filesToRename;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            filesToRename.push_back(entry.path());
        }
    }

    if (filesToRename.empty()) {
        std::cout << "No files found in the directory.\n";
        return;
    }

    std::cout << "Found " << filesToRename.size() << " file(s).\n";
    std::cout << "Pattern: '" << pattern << "' -> Replacement: '" << replacement << "'\n";
    std::cout << "Proceed with renaming? (y/n): ";

    char confirm;
    std::cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        std::cout << "Operation cancelled.\n";
        return;
    }

    int renamedCount = 0;
    for (const auto& oldPath : filesToRename) {
        std::string oldName = oldPath.filename().string();
        std::string newName = std::regex_replace(oldName, regexPattern, replacement);

        if (oldName != newName) {
            fs::path newPath = oldPath.parent_path() / newName;
            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldName << " -> " << newName << '\n';
                ++renamedCount;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldName << ": " << e.what() << '\n';
            }
        }
    }

    std::cout << "Renaming complete. " << renamedCount << " file(s) renamed.\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <regex_pattern> <replacement_string>\n";
        std::cerr << "Example: " << argv[0] << " ./photos \"image_(\\\\d+)\\.jpg\" \"photo_$1.jpg\"\n";
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(targetDir, pattern, replacement);
    return 0;
}