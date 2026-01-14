
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

namespace fs = std::filesystem;

class DirectoryWatcher {
private:
    fs::path watch_path;
    std::unordered_set<std::string> current_files;
    
    void refresh_file_list() {
        std::unordered_set<std::string> new_files;
        for (const auto& entry : fs::directory_iterator(watch_path)) {
            if (fs::is_regular_file(entry.path())) {
                new_files.insert(entry.path().filename().string());
            }
        }
        current_files = std::move(new_files);
    }
    
    void detect_changes(const std::unordered_set<std::string>& previous) {
        for (const auto& file : current_files) {
            if (previous.find(file) == previous.end()) {
                std::cout << "File added: " << file << std::endl;
            }
        }
        
        for (const auto& file : previous) {
            if (current_files.find(file) == current_files.end()) {
                std::cout << "File removed: " << file << std::endl;
            }
        }
    }

public:
    explicit DirectoryWatcher(const std::string& path) : watch_path(path) {
        if (!fs::exists(watch_path) || !fs::is_directory(watch_path)) {
            throw std::runtime_error("Invalid directory path");
        }
        refresh_file_list();
    }
    
    void start_monitoring(int interval_seconds = 2) {
        std::cout << "Monitoring directory: " << watch_path.string() << std::endl;
        
        while (true) {
            auto previous_files = current_files;
            refresh_file_list();
            detect_changes(previous_files);
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
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