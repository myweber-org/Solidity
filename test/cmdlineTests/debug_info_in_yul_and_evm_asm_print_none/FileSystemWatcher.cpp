#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_to_watch;
    std::chrono::duration<int, std::milli> interval;
    bool running;

public:
    FileSystemWatcher(const std::string& path, int interval_ms = 2000)
        : directory_to_watch(path), interval(interval_ms), running(false) {
        if (!fs::exists(directory_to_watch) || !fs::is_directory(directory_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
    }

    void start() {
        running = true;
        std::cout << "Starting to watch directory: " << directory_to_watch << std::endl;

        std::map<std::string, fs::file_time_type> last_modified_times;
        for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
            if (fs::is_regular_file(entry.path())) {
                last_modified_times[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }

        while (running) {
            std::this_thread::sleep_for(interval);

            for (const auto& entry : fs::directory_iterator(directory_to_watch)) {
                if (!fs::is_regular_file(entry.path())) {
                    continue;
                }

                std::string file_path = entry.path().string();
                auto current_mod_time = fs::last_write_time(entry.path());

                if (last_modified_times.find(file_path) == last_modified_times.end()) {
                    std::cout << "New file detected: " << file_path << std::endl;
                    last_modified_times[file_path] = current_mod_time;
                } else if (last_modified_times[file_path] != current_mod_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    last_modified_times[file_path] = current_mod_time;
                }
            }

            std::vector<std::string> files_to_remove;
            for (const auto& [file_path, mod_time] : last_modified_times) {
                if (!fs::exists(file_path)) {
                    std::cout << "File deleted: " << file_path << std::endl;
                    files_to_remove.push_back(file_path);
                }
            }
            for (const auto& file_path : files_to_remove) {
                last_modified_times.erase(file_path);
            }
        }
    }

    void stop() {
        running = false;
        std::cout << "Stopped watching directory." << std::endl;
    }
};

int main() {
    try {
        FileSystemWatcher watcher("./test_directory");
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}