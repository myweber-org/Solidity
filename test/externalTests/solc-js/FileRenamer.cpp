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
    int count = 0;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string oldName = entry.path().filename().string();
            std::string newName = std::regex_replace(oldName, regexPattern, replacement);

            if (oldName != newName) {
                fs::path oldPath = entry.path();
                fs::path newPath = entry.path().parent_path() / newName;

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldName << " -> " << newName << std::endl;
                    ++count;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldName << ": " << e.what() << std::endl;
                }
            }
        }
    }

    std::cout << "Total files renamed: " << count << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <regex_pattern> <replacement_string>" << std::endl;
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string pattern(argv[2]);
    std::string replacement(argv[3]);

    renameFilesInDirectory(dirPath, pattern, replacement);

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
    explicit FileRenamer(const std::string& directory_path) : dir_path(directory_path) {}

    bool rename_files(const std::string& prefix, int start_number = 1) {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry);
            }
        }

        if (files.empty()) {
            std::cout << "No files found in the directory.\n";
            return true;
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        int current_number = start_number;
        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = prefix + std::to_string(current_number) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++current_number;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
                return false;
            }
        }

        std::cout << "Renaming completed successfully.\n";
        return true;
    }

private:
    std::string dir_path;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <prefix> [start_number]\n";
        std::cerr << "Example: " << argv[0] << " ./photos vacation_ 1\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int start_number = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory);
    if (!renamer.rename_files(prefix, start_number)) {
        return 1;
    }

    return 0;
}