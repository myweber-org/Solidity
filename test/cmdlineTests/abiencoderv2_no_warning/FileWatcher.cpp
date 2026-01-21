#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const std::string& filepath) : file_path_(filepath), last_mod_time_(getLastWriteTime()) {}

    void startMonitoring(int interval_seconds = 2) {
        std::cout << "Monitoring file: " << file_path_ << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            if (checkForChanges()) {
                std::cout << "File changed at: " << getCurrentTime() << std::endl;
            }
        }
    }

private:
    std::string file_path_;
    fs::file_time_type last_mod_time_;

    fs::file_time_type getLastWriteTime() const {
        if (fs::exists(file_path_)) {
            return fs::last_write_time(file_path_);
        }
        return fs::file_time_type::min();
    }

    bool checkForChanges() {
        auto current_mod_time = getLastWriteTime();
        if (current_mod_time != last_mod_time_) {
            last_mod_time_ = current_mod_time;
            return true;
        }
        return false;
    }

    std::string getCurrentTime() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return std::string(buffer);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    std::string file_to_watch = argv[1];
    if (!fs::exists(file_to_watch)) {
        std::cerr << "Error: File does not exist." << std::endl;
        return 1;
    }

    FileWatcher watcher(file_to_watch);
    watcher.startMonitoring();

    return 0;
}