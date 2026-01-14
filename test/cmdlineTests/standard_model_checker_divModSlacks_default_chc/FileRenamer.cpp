#include <iostream>
#include <filesystem>
#include <string>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& pattern, const std::string& replacement)
        : dir_path(directory), regex_pattern(pattern), repl_str(replacement) {}

    bool renameFiles() {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Invalid directory path.\n";
            return false;
        }

        std::regex pattern_regex;
        try {
            pattern_regex = std::regex(regex_pattern);
        } catch (const std::regex_error& e) {
            std::cerr << "Error: Invalid regex pattern.\n";
            return false;
        }

        std::vector<fs::path> files_to_rename;
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                files_to_rename.push_back(entry.path());
            }
        }

        int renamed_count = 0;
        for (const auto& old_path : files_to_rename) {
            std::string old_name = old_path.filename().string();
            std::string new_name = std::regex_replace(old_name, pattern_regex, repl_str);

            if (old_name != new_name) {
                fs::path new_path = old_path.parent_path() / new_name;
                try {
                    fs::rename(old_path, new_path);
                    std::cout << "Renamed: " << old_name << " -> " << new_name << std::endl;
                    ++renamed_count;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << old_name << ": " << e.what() << std::endl;
                }
            }
        }

        std::cout << "Total files renamed: " << renamed_count << std::endl;
        return renamed_count > 0;
    }

private:
    std::string dir_path;
    std::string regex_pattern;
    std::string repl_str;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <regex_pattern> <replacement>\n";
        return 1;
    }

    FileRenamer renamer(argv[1], argv[2], argv[3]);
    return renamer.renameFiles() ? 0 : 1;
}