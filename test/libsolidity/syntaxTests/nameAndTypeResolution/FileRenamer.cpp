
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <regex>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& pattern = ".*")
        : dir_path(directory), file_pattern(pattern) {}

    bool scanFiles() {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return false;
        }

        std::regex regex_pattern(file_pattern);
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                if (std::regex_match(filename, regex_pattern)) {
                    file_list.push_back(entry.path());
                }
            }
        }

        std::sort(file_list.begin(), file_list.end());
        return true;
    }

    void renameWithPrefix(int start_index = 1, int digits = 3) {
        if (file_list.empty()) {
            std::cout << "No files to rename." << std::endl;
            return;
        }

        int counter = start_index;
        for (const auto& old_path : file_list) {
            std::string extension = old_path.extension().string();
            std::string new_filename = formatNumber(counter, digits) + "_" + old_path.stem().string() + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
            }
        }
    }

    void listFiles() const {
        if (file_list.empty()) {
            std::cout << "File list is empty." << std::endl;
            return;
        }
        std::cout << "Files matching pattern:" << std::endl;
        for (const auto& path : file_list) {
            std::cout << "  " << path.filename() << std::endl;
        }
    }

private:
    std::string formatNumber(int num, int width) {
        std::string result = std::to_string(num);
        while (result.length() < static_cast<size_t>(width)) {
            result = "0" + result;
        }
        return result;
    }

    fs::path dir_path;
    std::string file_pattern;
    std::vector<fs::path> file_list;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory> [pattern]" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string pattern = (argc > 2) ? argv[2] : ".*";

    FileRenamer renamer(directory, pattern);
    if (!renamer.scanFiles()) {
        return 1;
    }

    renamer.listFiles();
    
    std::cout << "\nProceed with renaming? (y/n): ";
    char choice;
    std::cin >> choice;
    
    if (choice == 'y' || choice == 'Y') {
        renamer.renameWithPrefix();
        std::cout << "Renaming completed." << std::endl;
    } else {
        std::cout << "Renaming cancelled." << std::endl;
    }

    return 0;
}