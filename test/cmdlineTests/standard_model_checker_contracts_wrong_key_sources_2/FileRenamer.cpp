
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix)
        : target_directory(directory), name_prefix(prefix) {}

    bool renameFiles() {
        if (!fs::exists(target_directory) || !fs::is_directory(target_directory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(target_directory)) {
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

        int counter = 1;
        for (const auto& file : files) {
            fs::path old_path = file.path();
            std::string extension = old_path.extension().string();

            std::string new_filename = name_prefix + "_" + std::to_string(counter) + extension;
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
            }
        }

        std::cout << "Renaming process completed. " << (counter - 1) << " files processed.\n";
        return true;
    }

private:
    std::string target_directory;
    std::string name_prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <name_prefix>\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];

    FileRenamer renamer(directory, prefix);
    return renamer.renameFiles() ? 0 : 1;
}
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFiles(const std::string& directory, const std::string& baseName) {
        std::vector<fs::path> files;
        
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                files.push_back(entry.path());
            }
        }
        
        std::sort(files.begin(), files.end());
        
        int counter = 1;
        for (const auto& file : files) {
            std::string extension = file.extension().string();
            std::string newName = baseName + "_" + std::to_string(counter) + extension;
            fs::path newPath = file.parent_path() / newName;
            
            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newName << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.filename() << ": " << e.what() << std::endl;
            }
        }
        
        std::cout << "Total files renamed: " << counter - 1 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <base_name>" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string baseName = argv[2];
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path" << std::endl;
        return 1;
    }
    
    FileRenamer::renameFiles(directory, baseName);
    
    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

void renameFilesWithTimestamp(const fs::path& directory) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestamp_stream;
    timestamp_stream << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S_");
    std::string timestamp_prefix = timestamp_stream.str();

    int renamed_count = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            fs::path old_path = entry.path();
            std::string new_filename = timestamp_prefix + old_path.filename().string();
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_path.filename() << std::endl;
                ++renamed_count;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
            }
        }
    }

    std::cout << "Total files renamed: " << renamed_count << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path target_dir(argv[1]);
    renameFilesWithTimestamp(target_dir);

    return 0;
}