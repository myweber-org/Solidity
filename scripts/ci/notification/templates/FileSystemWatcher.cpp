#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

void watchDirectory(const fs::path& directoryPath) {
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        std::cerr << "Error: Provided path is not a valid directory." << std::endl;
        return;
    }

    std::cout << "Watching directory: " << directoryPath << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    auto lastWriteTime = fs::last_write_time(directoryPath);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        try {
            auto currentWriteTime = fs::last_write_time(directoryPath);

            if (currentWriteTime != lastWriteTime) {
                lastWriteTime = currentWriteTime;
                std::cout << "Directory modified at: "
                          << std::chrono::system_clock::to_time_t(
                                 std::chrono::file_clock::to_sys(currentWriteTime))
                          << std::endl;

                for (const auto& entry : fs::directory_iterator(directoryPath)) {
                    std::cout << "  Found: " << entry.path().filename() << std::endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            break;
        } catch (...) {
            std::cerr << "Unknown error occurred." << std::endl;
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    fs::path dirPath = argv[1];
    watchDirectory(dirPath);

    return 0;
}