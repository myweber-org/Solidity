
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t creation_time;
};

bool compareByCreationTime(const FileInfo& a, const FileInfo& b) {
    return a.creation_time < b.creation_time;
}

void renameFilesSequentially(const fs::path& directory, const std::string& prefix) {
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
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return;
    }

    std::sort(files.begin(), files.end(), compareByCreationTime);

    int counter = 1;
    for (const auto& file : files) {
        std::string extension = file.path.extension().string();
        fs::path new_name = directory / (prefix + "_" + std::to_string(counter) + extension);

        try {
            fs::rename(file.path, new_name);
            std::cout << "Renamed: " << file.path.filename() << " -> " << new_name.filename() << '\n';
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << file.path << ": " << e.what() << '\n';
        }
    }

    std::cout << "Processed " << (counter - 1) << " files.\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <prefix>\n";
        return 1;
    }

    fs::path target_dir(argv[1]);
    std::string prefix(argv[2]);

    if (!fs::exists(target_dir) || !fs::is_directory(target_dir)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    renameFilesSequentially(target_dir, prefix);
    return 0;
}