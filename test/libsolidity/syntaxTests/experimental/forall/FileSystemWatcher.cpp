#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running = false;

    void populateFileMap() {
        file_timestamps.clear();
        if (fs::exists(path_to_watch) && fs::is_directory(path_to_watch)) {
            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    file_timestamps[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch)) {
            throw std::runtime_error("Path does not exist: " + path);
        }
        populateFileMap();
    }

    void start(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << path_to_watch << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            if (!fs::exists(path_to_watch)) {
                std::cerr << "Directory no longer exists. Stopping watcher." << std::endl;
                stop();
                break;
            }

            for (const auto& entry : fs::directory_iterator(path_to_watch)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string file_path = entry.path().string();
                    auto current_time = fs::last_write_time(entry.path());

                    if (file_timestamps.find(file_path) == file_timestamps.end()) {
                        std::cout << "File created: " << file_path << std::endl;
                        file_timestamps[file_path] = current_time;
                    } else if (file_timestamps[file_path] != current_time) {
                        std::cout << "File modified: " << file_path << std::endl;
                        file_timestamps[file_path] = current_time;
                    }
                }
            }

            for (auto it = file_timestamps.begin(); it != file_timestamps.end(); ) {
                if (!fs::exists(it->first)) {
                    std::cout << "File deleted: " << it->first << std::endl;
                    it = file_timestamps.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}