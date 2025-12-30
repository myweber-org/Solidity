#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

void watch_directory(const fs::path& dir_path, int interval_seconds = 2) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cerr << "Error: Path is not a valid directory." << std::endl;
        return;
    }

    std::cout << "Watching directory: " << dir_path << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    auto last_write_time = fs::last_write_time(dir_path);
    auto file_count = std::distance(fs::directory_iterator(dir_path), fs::directory_iterator{});

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

        try {
            auto current_write_time = fs::last_write_time(dir_path);
            auto current_file_count = std::distance(fs::directory_iterator(dir_path), fs::directory_iterator{});

            if (current_write_time != last_write_time) {
                std::cout << "[Modified] Directory timestamp changed." << std::endl;
                last_write_time = current_write_time;
            }

            if (current_file_count > file_count) {
                std::cout << "[New] File(s) added. Total files: " << current_file_count << std::endl;
                file_count = current_file_count;
            } else if (current_file_count < file_count) {
                std::cout << "[Deleted] File(s) removed. Total files: " << current_file_count << std::endl;
                file_count = current_file_count;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            break;
        } catch (...) {
            std::cerr << "Unknown error occurred while watching directory." << std::endl;
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    fs::path watch_path = fs::current_path();

    if (argc > 1) {
        watch_path = argv[1];
    }

    watch_directory(watch_path);
    return 0;
}