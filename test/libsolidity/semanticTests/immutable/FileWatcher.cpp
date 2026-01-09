#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileWatcher {
public:
    explicit FileWatcher(const fs::path& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        update_file_list();
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring directory: " << watch_path << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_changes();
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> current_files;

    void update_file_list() {
        current_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                current_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_changes() {
        auto previous_files = current_files;
        update_file_list();

        for (const auto& file : current_files) {
            if (previous_files.find(file) == previous_files.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : previous_files) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}