
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    static void renameFilesInDirectory(const fs::path& directory,
                                       const std::string& prefix,
                                       const std::string& extension) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.path())) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string newFilename = prefix + std::to_string(counter) + extension;
            fs::path newPath = directory / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. Total files processed: " << counter - 1 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos image_ .jpg" << std::endl;
        return 1;
    }

    fs::path dirPath(argv[1]);
    std::string prefix(argv[2]);
    std::string extension(argv[3]);

    FileRenamer::renameFilesInDirectory(dirPath, prefix, extension);

    return 0;
}#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t mod_time;
};

bool compareByModTime(const FileInfo& a, const FileInfo& b) {
    return a.mod_time < b.mod_time;
}

void renameFilesInDirectory(const fs::path& directory, const std::string& prefix) {
    std::vector<FileInfo> files;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                auto mod_time = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    mod_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                
                files.push_back({entry.path(), cftime});
            }
        }

        std::sort(files.begin(), files.end(), compareByModTime);

        int counter = 1;
        for (const auto& file : files) {
            std::string extension = file.path.extension().string();
            fs::path new_name = directory / (prefix + "_" + std::to_string(counter) + extension);
            
            try {
                fs::rename(file.path, new_name);
                std::cout << "Renamed: " << file.path.filename() << " -> " << new_name.filename() << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << file.path << ": " << e.what() << std::endl;
            }
            
            counter++;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        return 1;
    }

    fs::path target_dir(argv[1]);
    std::string prefix(argv[2]);

    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        std::cerr << "Invalid directory: " << target_dir << std::endl;
        return 1;
    }

    renameFilesInDirectory(target_dir, prefix);
    return 0;
}