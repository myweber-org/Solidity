#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t creation_time;
};

void renameFilesSequentially(const std::string& directory, const std::string& prefix) {
    std::vector<FileInfo> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            auto ftime = fs::last_write_time(entry.path());
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
            
            files.push_back({entry.path(), ctime});
        }
    }
    
    std::sort(files.begin(), files.end(), [](const FileInfo& a, const FileInfo& b) {
        return a.creation_time < b.creation_time;
    });
    
    int counter = 1;
    for (const auto& file : files) {
        std::string extension = file.path.extension().string();
        fs::path new_path = fs::path(directory) / (prefix + "_" + std::to_string(counter++) + extension);
        
        try {
            fs::rename(file.path, new_path);
            std::cout << "Renamed: " << file.path.filename() << " -> " << new_path.filename() << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << file.path << ": " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix>" << std::endl;
        return 1;
    }
    
    std::string directory = argv[1];
    std::string prefix = argv[2];
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Invalid directory: " << directory << std::endl;
        return 1;
    }
    
    renameFilesSequentially(directory, prefix);
    return 0;
}#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& prefix) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Provided path is not a valid directory." << std::endl;
        return;
    }

    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            entries.push_back(entry);
        }
    }

    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename().string() < b.path().filename().string();
              });

    int counter = 1;
    for (const auto& entry : entries) {
        fs::path oldPath = entry.path();
        std::string extension = oldPath.extension().string();
        std::string newFilename = prefix + "_" + std::to_string(counter) + extension;
        fs::path newPath = oldPath.parent_path() / newFilename;

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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);

    renameFilesInDirectory(targetDir, prefix);
    return 0;
}