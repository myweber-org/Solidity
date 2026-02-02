
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
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

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>\n";
        return 1;
    }

    fs::path target_dir(argv[1]);
    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    std::vector<FileInfo> files;
    try {
        for (const auto& entry : fs::directory_iterator(target_dir)) {
            if (fs::is_regular_file(entry.status())) {
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
                files.push_back({entry.path(), ctime});
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return 1;
    }

    if (files.empty()) {
        std::cout << "No regular files found in the directory.\n";
        return 0;
    }

    std::sort(files.begin(), files.end(), compareByCreationTime);

    int counter = 1;
    for (const auto& file_info : files) {
        std::stringstream new_name;
        new_name << std::setw(3) << std::setfill('0') << counter << "_" << file_info.path.filename().string();
        fs::path new_path = file_info.path.parent_path() / new_name.str();

        try {
            fs::rename(file_info.path, new_path);
            std::cout << "Renamed: " << file_info.path.filename() << " -> " << new_name.str() << '\n';
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << file_info.path.filename() << ": " << e.what() << '\n';
        }
        ++counter;
    }

    return 0;
}