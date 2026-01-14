
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

struct FileInfo {
    fs::path path;
    std::time_t mod_time;
};

bool compareByModTime(const FileInfo& a, const FileInfo& b) {
    return a.mod_time < b.mod_time;
}

void renameFilesSequentially(const fs::path& directory, const std::string& prefix) {
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
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
        return;
    }

    if (files.empty()) {
        std::cout << "No files found in directory." << std::endl;
        return;
    }

    std::sort(files.begin(), files.end(), compareByModTime);

    int counter = 1;
    for (const auto& file : files) {
        std::stringstream new_name;
        new_name << prefix << std::setw(4) << std::setfill('0') << counter << file.path().extension().string();
        
        fs::path new_path = file.path().parent_path() / new_name.str();
        
        try {
            fs::rename(file.path(), new_path);
            std::cout << "Renamed: " << file.path().filename() << " -> " << new_name.str() << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << file.path().filename() << ": " << e.what() << std::endl;
        }
        
        counter++;
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
        std::cerr << "Invalid directory path." << std::endl;
        return 1;
    }

    renameFilesSequentially(target_dir, prefix);
    return 0;
}