#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class DirectoryWatcher {
private:
    fs::path watchPath;
    std::unordered_set<std::string> previousFiles;

    std::unordered_set<std::string> getCurrentFiles() {
        std::unordered_set<std::string> currentFiles;
        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            for (const auto& entry : fs::directory_iterator(watchPath)) {
                if (fs::is_regular_file(entry.path())) {
                    currentFiles.insert(entry.path().filename().string());
                }
            }
        }
        return currentFiles;
    }

public:
    DirectoryWatcher(const std::string& path) : watchPath(path) {
        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            previousFiles = getCurrentFiles();
            std::cout << "Watching directory: " << watchPath.string() << std::endl;
        } else {
            throw std::runtime_error("Invalid directory path");
        }
    }

    void checkChanges() {
        auto currentFiles = getCurrentFiles();

        for (const auto& file : currentFiles) {
            if (previousFiles.find(file) == previousFiles.end()) {
                std::cout << "New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : previousFiles) {
            if (currentFiles.find(file) == currentFiles.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }

        previousFiles = currentFiles;
    }

    void startMonitoring(int intervalSeconds = 2) {
        std::cout << "Starting monitoring (interval: " << intervalSeconds << "s)" << std::endl;
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            
            try {
                checkChanges();
            } catch (const std::exception& e) {
                std::cerr << "Error checking directory: " << e.what() << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        DirectoryWatcher watcher(argv[1]);
        watcher.startMonitoring();
    } catch (const std::exception& e) {
        std::cerr << "Failed to start watcher: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_map<std::string, fs::file_time_type> file_timestamps;
    bool running;

    void populate_timestamps() {
        file_timestamps.clear();
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                file_timestamps[entry.path().filename().string()] = fs::last_write_time(entry.path());
            }
        }
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Provided path is not a valid directory.");
        }
        populate_timestamps();
    }

    void start_watching(int interval_seconds = 1) {
        running = true;
        std::cout << "Watching directory: " << directory_path << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            for (const auto& entry : fs::directory_iterator(directory_path)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string filename = entry.path().filename().string();
                    auto current_time = fs::last_write_time(entry.path());

                    if (file_timestamps.find(filename) == file_timestamps.end()) {
                        std::cout << "New file detected: " << filename << std::endl;
                        file_timestamps[filename] = current_time;
                    } else if (file_timestamps[filename] != current_time) {
                        std::cout << "File modified: " << filename << std::endl;
                        file_timestamps[filename] = current_time;
                    }
                }
            }

            std::vector<std::string> files_to_remove;
            for (const auto& [filename, _] : file_timestamps) {
                fs::path file_path = directory_path / filename;
                if (!fs::exists(file_path)) {
                    std::cout << "File deleted: " << filename << std::endl;
                    files_to_remove.push_back(filename);
                }
            }

            for (const auto& filename : files_to_remove) {
                file_timestamps.erase(filename);
            }
        }
    }

    void stop_watching() {
        running = false;
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.start_watching(2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}