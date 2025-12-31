
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t mod_time;
};

bool compareByModTime(const FileInfo& a, const FileInfo& b) {
    return a.mod_time < b.mod_time;
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
            if (fs::is_regular_file(entry.path())) {
                auto mod_time = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    mod_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
                std::time_t c_time = std::chrono::system_clock::to_time_t(sctp);
                files.push_back({entry.path(), c_time});
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return 1;
    }

    std::sort(files.begin(), files.end(), compareByModTime);

    int counter = 1;
    for (const auto& file_info : files) {
        fs::path old_path = file_info.path;
        std::string extension = old_path.extension().string();
        fs::path new_path = old_path.parent_path() / (std::to_string(counter) + extension);

        try {
            fs::rename(old_path, new_path);
            std::cout << "Renamed: " << old_path.filename() << " -> " << new_path.filename() << '\n';
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << '\n';
        }
        ++counter;
    }

    std::cout << "File renaming completed.\n";
    return 0;
}