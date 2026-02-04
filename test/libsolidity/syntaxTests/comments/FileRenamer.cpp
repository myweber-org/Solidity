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
    int renameCount = 0;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string oldName = entry.path().filename().string();
                std::string newName = std::regex_replace(oldName, regexPattern, replacement);

                if (oldName != newName && !newName.empty()) {
                    fs::path oldPath = entry.path();
                    fs::path newPath = oldPath.parent_path() / newName;

                    if (!fs::exists(newPath)) {
                        fs::rename(oldPath, newPath);
                        std::cout << "Renamed: " << oldName << " -> " << newName << std::endl;
                        ++renameCount;
                    } else {
                        std::cerr << "Warning: Skipping '" << oldName << "' because '" << newName << "' already exists." << std::endl;
                    }
                }
            }
        }
        std::cout << "Total files renamed: " << renameCount << std::endl;
    } catch (const std::regex_error& e) {
        std::cerr << "Error: Invalid regex pattern. " << e.what() << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <regex_pattern> <replacement_string>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./myfiles \"(.*)\\.txt\" \"$1_backup.txt\"" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(targetDir, pattern, replacement);

    return 0;
}