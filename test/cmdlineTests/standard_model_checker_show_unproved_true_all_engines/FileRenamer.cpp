
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

void renameFilesSequentially(const std::string& directory, const std::string& prefix) {
    std::vector<FileInfo> files;
    
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t mod_time = std::chrono::system_clock::to_time_t(sctp);
                
                files.push_back({entry.path(), mod_time});
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return;
    }
    
    std::sort(files.begin(), files.end(), [](const FileInfo& a, const FileInfo& b) {
        return a.mod_time < b.mod_time;
    });
    
    int counter = 1;
    for (const auto& file : files) {
        std::string extension = file.path.extension().string();
        fs::path new_path = file.path.parent_path() / (prefix + "_" + 
                          std::to_string(counter) + extension);
        
        try {
            fs::rename(file.path, new_path);
            std::cout << "Renamed: " << file.path.filename() << " -> " 
                      << new_path.filename() << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to rename " << file.path.filename() 
                      << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Processed " << (counter - 1) << " files." << std::endl;
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
}