
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <ctime>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t creation_time;
};

bool compareByCreationTime(const FileInfo& a, const FileInfo& b) {
    return a.creation_time < b.creation_time;
}

void renameFilesInDirectory(const fs::path& directory, const std::string& prefix) {
    std::vector<FileInfo> files;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                auto ftime = fs::last_write_time(entry.path());
                auto sct = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t creation_time = std::chrono::system_clock::to_time_t(sct);

                files.push_back({entry.path(), creation_time});
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return;
    }

    if (files.empty()) {
        std::cout << "No files found in directory." << std::endl;
        return;
    }

    std::sort(files.begin(), files.end(), compareByCreationTime);

    int counter = 1;
    for (const auto& fileInfo : files) {
        std::string extension = fileInfo.path.extension().string();
        fs::path newPath = directory / (prefix + "_" + std::to_string(counter) + extension);

        try {
            fs::rename(fileInfo.path, newPath);
            std::cout << "Renamed: " << fileInfo.path.filename() << " -> " << newPath.filename() << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << fileInfo.path << ": " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    std::string prefix(argv[2]);

    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        std::cerr << "Invalid directory path." << std::endl;
        return 1;
    }

    renameFilesInDirectory(targetDir, prefix);
    return 0;
}