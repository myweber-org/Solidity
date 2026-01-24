
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t creation_time;
};

bool compareByCreationTime(const FileInfo& a, const FileInfo& b) {
    return a.creation_time < b.creation_time;
}

std::string formatTime(std::time_t time) {
    std::tm* tm_info = std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(tm_info, "%Y%m%d_%H%M%S");
    return oss.str();
}

void renameFilesInDirectory(const fs::path& directory, const std::string& prefix) {
    std::vector<FileInfo> files;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t creation_time = std::chrono::system_clock::to_time_t(sctp);

                files.push_back({entry.path(), creation_time});
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return;
    }

    if (files.empty()) {
        std::cout << "No files found in directory: " << directory << std::endl;
        return;
    }

    std::sort(files.begin(), files.end(), compareByCreationTime);

    int counter = 1;
    for (const auto& fileInfo : files) {
        std::string extension = fileInfo.path.extension().string();
        std::string newFilename = prefix + "_" + formatTime(fileInfo.creation_time) + 
                                 "_" + std::to_string(counter) + extension;
        
        fs::path newPath = fileInfo.path.parent_path() / newFilename;

        try {
            fs::rename(fileInfo.path, newPath);
            std::cout << "Renamed: " << fileInfo.path.filename() << " -> " << newFilename << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << fileInfo.path.filename() << ": " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <prefix>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos vacation" << std::endl;
        return 1;
    }

    fs::path directory(argv[1]);
    std::string prefix(argv[2]);

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Invalid directory: " << directory << std::endl;
        return 1;
    }

    renameFilesInDirectory(directory, prefix);
    return 0;
}