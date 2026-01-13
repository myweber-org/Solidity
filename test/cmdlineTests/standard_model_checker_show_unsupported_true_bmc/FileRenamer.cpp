
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