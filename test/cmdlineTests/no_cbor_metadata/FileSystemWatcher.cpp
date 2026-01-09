
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path path_to_watch;
    std::unordered_set<std::string> current_files;

    std::unordered_set<std::string> get_files_in_directory() {
        std::unordered_set<std::string> files;
        for (const auto& entry : fs::directory_iterator(path_to_watch)) {
            if (entry.is_regular_file()) {
                files.insert(entry.path().filename().string());
            }
        }
        return files;
    }

    void compare_and_notify(const std::unordered_set<std::string>& new_files) {
        // Check for added files
        for (const auto& file : new_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }

        // Check for removed files
        for (const auto& file : current_files) {
            if (new_files.find(file) == new_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : path_to_watch(path) {
        if (!fs::exists(path_to_watch) || !fs::is_directory(path_to_watch)) {
            throw std::invalid_argument("Provided path is not a valid directory.");
        }
        current_files = get_files_in_directory();
        std::cout << "Watching directory: " << fs::absolute(path_to_watch) << std::endl;
        std::cout << "Initial file count: " << current_files.size() << std::endl;
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Starting monitoring (interval: " << interval_seconds << "s). Press Ctrl+C to stop." << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto new_files = get_files_in_directory();
            compare_and_notify(new_files);
            current_files = new_files;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <functional>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    using Callback = std::function<void(const fs::path&, const std::string&)>;

    FileSystemWatcher(const fs::path& directory, Callback callback)
        : watch_directory(directory), change_callback(callback), running(false) {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        refresh_file_list();
    }

    ~FileSystemWatcher() {
        stop();
    }

    void start() {
        running = true;
        monitor_thread = std::thread(&FileSystemWatcher::monitor_loop, this);
    }

    void stop() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }

private:
    fs::path watch_directory;
    Callback change_callback;
    std::unordered_set<std::string> known_files;
    bool running;
    std::thread monitor_thread;

    void refresh_file_list() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void monitor_loop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            std::unordered_set<std::string> current_files;
            for (const auto& entry : fs::directory_iterator(watch_directory)) {
                if (entry.is_regular_file()) {
                    current_files.insert(entry.path().filename().string());
                }
            }

            for (const auto& file : current_files) {
                if (known_files.find(file) == known_files.end()) {
                    change_callback(watch_directory / file, "created");
                }
            }

            for (const auto& file : known_files) {
                if (current_files.find(file) == current_files.end()) {
                    change_callback(watch_directory / file, "deleted");
                }
            }

            known_files = std::move(current_files);
        }
    }
};

void example_callback(const fs::path& file_path, const std::string& action) {
    std::cout << "File " << file_path.filename() << " was " << action << " at "
              << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
}

int main() {
    try {
        FileSystemWatcher watcher("./test_dir", example_callback);
        watcher.start();

        std::cout << "Watching directory './test_dir'. Press Enter to stop..." << std::endl;
        std::cin.get();

        watcher.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}