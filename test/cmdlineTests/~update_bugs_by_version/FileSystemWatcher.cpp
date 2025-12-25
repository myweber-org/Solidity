
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class DirectoryWatcher {
public:
    explicit DirectoryWatcher(const std::string& path) : watch_path(path) {
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            refresh_file_list();
        }
    }

    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring directory: " << watch_path << std::endl;
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto current_files = get_current_files();
            detect_changes(current_files);
            file_set = std::move(current_files);
        }
    }

private:
    fs::path watch_path;
    std::unordered_set<std::string> file_set;

    void refresh_file_list() {
        file_set.clear();
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            file_set.insert(entry.path().filename().string());
        }
    }

    std::unordered_set<std::string> get_current_files() {
        std::unordered_set<std::string> current;
        if (fs::exists(watch_path) && fs::is_directory(watch_path)) {
            for (const auto& entry : fs::directory_iterator(watch_path)) {
                current.insert(entry.path().filename().string());
            }
        }
        return current;
    }

    void detect_changes(const std::unordered_set<std::string>& current) {
        for (const auto& file : current) {
            if (file_set.find(file) == file_set.end()) {
                std::cout << "[+] New file detected: " << file << std::endl;
            }
        }

        for (const auto& file : file_set) {
            if (current.find(file) == current.end()) {
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
        DirectoryWatcher watcher(argv[1]);
        watcher.start_monitoring();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}