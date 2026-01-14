#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileWatcher {
public:
    FileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        update_file_states();
    }

    void start_watching(int interval_seconds = 1) {
        std::cout << "Watching directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_map<std::string, fs::file_time_type> file_states;

    void update_file_states() {
        file_states.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                file_states[entry.path().string()] = fs::last_write_time(entry);
            }
        }
    }

    void check_for_changes() {
        for (const auto& entry : fs::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                auto current_time = fs::last_write_time(entry);

                if (file_states.find(file_path) == file_states.end()) {
                    std::cout << "New file detected: " << file_path << std::endl;
                    file_states[file_path] = current_time;
                } else if (file_states[file_path] != current_time) {
                    std::cout << "File modified: " << file_path << std::endl;
                    file_states[file_path] = current_time;
                }
            }
        }

        std::vector<std::string> files_to_remove;
        for (const auto& [file_path, _] : file_states) {
            if (!fs::exists(file_path)) {
                std::cout << "File deleted: " << file_path << std::endl;
                files_to_remove.push_back(file_path);
            }
        }

        for (const auto& file_path : files_to_remove) {
            file_states.erase(file_path);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileWatcher watcher(argv[1]);
        watcher.start_watching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}