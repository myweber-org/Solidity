#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

void renameFilesWithTimestamp(const fs::path& directory) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Provided path is not a valid directory." << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);
    std::ostringstream timestamp_stream;
    timestamp_stream << std::put_time(&tm_buf, "%Y%m%d_%H%M%S");
    std::string timestamp = timestamp_stream.str();

    int count = 0;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.status())) {
            fs::path old_path = entry.path();
            std::string new_filename = timestamp + "_" + old_path.filename().string();
            fs::path new_path = old_path.parent_path() / new_filename;

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_filename << std::endl;
                ++count;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << old_path.filename() << ": " << e.what() << std::endl;
            }
        }
    }

    std::cout << "Total files renamed: " << count << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path target_dir(argv[1]);
    renameFilesWithTimestamp(target_dir);
    return 0;
}