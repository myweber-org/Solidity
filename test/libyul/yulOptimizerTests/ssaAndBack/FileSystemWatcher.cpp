#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

void watchDirectory(const fs::path& dirPath) {
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error: Path is not a valid directory." << std::endl;
        return;
    }

    std::cout << "Watching directory: " << dirPath << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    auto lastWriteTime = fs::last_write_time(dirPath);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        try {
            auto currentWriteTime = fs::last_write_time(dirPath);

            if (currentWriteTime != lastWriteTime) {
                std::cout << "Directory modified at: "
                          << std::chrono::system_clock::to_time_t(
                                 std::chrono::file_clock::to_sys(currentWriteTime))
                          << std::endl;
                lastWriteTime = currentWriteTime;
            }

            for (const auto& entry : fs::directory_iterator(dirPath)) {
                std::cout << "Found: " << entry.path().filename() << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
}

int main() {
    fs::path pathToWatch = "./test_watch";
    watchDirectory(pathToWatch);
    return 0;
}