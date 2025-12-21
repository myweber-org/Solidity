#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

bool renameFilesWithTimestamp(const fs::path& directory) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Provided path is not a valid directory." << std::endl;
        return false;
    }

    std::string timestamp = getCurrentTimestamp();
    int renameCount = 0;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry.status())) {
                fs::path oldPath = entry.path();
                std::string newFilename = timestamp + "_" + oldPath.filename().string();
                fs::path newPath = oldPath.parent_path() / newFilename;

                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                renameCount++;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }

    std::cout << "Total files renamed: " << renameCount << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path targetDir(argv[1]);
    if (!renameFilesWithTimestamp(targetDir)) {
        return 1;
    }

    return 0;
}