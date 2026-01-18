
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& filepath) : file_to_watch(filepath), last_write_time(get_last_write_time()) {}

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching file: " << file_to_watch << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(file_to_watch)) {
                log_message("File has been deleted.");
                break;
            }

            auto current_write_time = get_last_write_time();
            if (current_write_time != last_write_time) {
                last_write_time = current_write_time;
                log_message("File has been modified.");
            }
        }
    }

private:
    fs::path file_to_watch;
    fs::file_time_type last_write_time;

    fs::file_time_type get_last_write_time() const {
        if (fs::exists(file_to_watch)) {
            return fs::last_write_time(file_to_watch);
        }
        return fs::file_time_type::min();
    }

    void log_message(const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] " << msg << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    fs::path file_path(argv[1]);
    if (!fs::exists(file_path)) {
        std::cerr << "Error: File '" << file_path << "' does not exist." << std::endl;
        return 1;
    }

    FileWatcher watcher(file_path);
    watcher.start_watching();

    return 0;
}