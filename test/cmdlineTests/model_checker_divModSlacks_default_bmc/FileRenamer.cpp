#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

void renameFilesWithTimestamp(const std::string& directoryPath) {
    try {
        fs::path dir(directoryPath);
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        localtime_s(&tm_buf, &in_time_t);
        std::ostringstream timestamp;
        timestamp << std::put_time(&tm_buf, "%Y%m%d_%H%M%S_");

        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry.status())) {
                fs::path oldPath = entry.path();
                std::string newFilename = timestamp.str() + oldPath.filename().string();
                fs::path newPath = oldPath.parent_path() / newFilename;

                try {
                    fs::rename(oldPath, newPath);
                    std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
                }
            }
        }
        std::cout << "Batch renaming completed." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }
    renameFilesWithTimestamp(argv[1]);
    return 0;
}