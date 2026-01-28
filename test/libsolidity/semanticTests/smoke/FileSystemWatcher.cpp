#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> last_snapshot;

    std::unordered_set<std::string> take_snapshot() {
        std::unordered_set<std::string> current_files;
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                if (fs::is_regular_file(entry.status())) {
                    current_files.insert(entry.path().filename().string());
                }
            }
        }
        return current_files;
    }

    void report_changes(const std::unordered_set<std::string>& current) {
        for (const auto& file : current) {
            if (!last_snapshot.count(file)) {
                std::cout << "[+] File added: " << file << std::endl;
            }
        }
        for (const auto& file : last_snapshot) {
            if (!current.count(file)) {
                std::cout << "[-] File removed: " << file << std::endl;
            }
        }
    }

public:
    explicit FileSystemWatcher(const std::string& path) : watch_path(path) {
        if (!fs::exists(watch_path)) {
            throw std::runtime_error("Path does not exist");
        }
        last_snapshot = take_snapshot();
        std::cout << "Watching directory: " << fs::absolute(watch_path) << std::endl;
    }

    void poll(int interval_seconds = 2) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            auto current = take_snapshot();
            report_changes(current);
            last_snapshot = std::move(current);
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
        watcher.poll();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}