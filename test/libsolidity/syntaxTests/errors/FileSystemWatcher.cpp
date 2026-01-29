#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
private:
    fs::path directory_path;
    std::unordered_set<std::string> known_files;
    bool running;

    std::unordered_set<std::string> get_current_files() {
        std::unordered_set<std::string> current;
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.path())) {
                current.insert(entry.path().filename().string());
            }
        }
        return current;
    }

public:
    FileSystemWatcher(const std::string& path) : directory_path(path), running(false) {
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::invalid_argument("Provided path is not a valid directory");
        }
        known_files = get_current_files();
    }

    void start_monitoring(int interval_seconds = 2) {
        running = true;
        std::cout << "Monitoring directory: " << directory_path << std::endl;
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            
            auto current_files = get_current_files();
            
            // Detect new files
            for (const auto& file : current_files) {
                if (known_files.find(file) == known_files.end()) {
                    std::cout << "[NEW] File detected: " << file << std::endl;
                }
            }
            
            // Detect deleted files
            for (const auto& file : known_files) {
                if (current_files.find(file) == current_files.end()) {
                    std::cout << "[DELETED] File removed: " << file << std::endl;
                }
            }
            
            known_files = current_files;
        }
    }

    void stop_monitoring() {
        running = false;
        std::cout << "Monitoring stopped." << std::endl;
    }

    void print_current_state() {
        std::cout << "Currently tracked files (" << known_files.size() << "):" << std::endl;
        for (const auto& file : known_files) {
            std::cout << "  - " << file << std::endl;
        }
    }
};

int main() {
    try {
        FileSystemWatcher watcher(".");
        watcher.print_current_state();
        
        std::thread monitor_thread([&watcher]() {
            watcher.start_monitoring();
        });
        
        std::cout << "Press Enter to stop monitoring..." << std::endl;
        std::cin.get();
        
        watcher.stop_monitoring();
        monitor_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_set>

namespace fs = boost::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path) {
        scan_files();
    }

    void start() {
        schedule_next_scan();
    }

private:
    void scan_files() {
        std::unordered_set<std::string> current_files;

        try {
            if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
                for (const auto& entry : fs::directory_iterator(watch_path_)) {
                    if (fs::is_regular_file(entry.path())) {
                        std::string filename = entry.path().string();
                        current_files.insert(filename);

                        if (previous_files_.find(filename) == previous_files_.end()) {
                            std::cout << "File created: " << filename << std::endl;
                        }
                    }
                }

                for (const auto& old_file : previous_files_) {
                    if (current_files.find(old_file) == current_files.end()) {
                        std::cout << "File deleted: " << old_file << std::endl;
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        previous_files_ = std::move(current_files);
    }

    void schedule_next_scan() {
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec) {
                scan_files();
                schedule_next_scan();
            }
        });
    }

    boost::asio::steady_timer timer_;
    std::string watch_path_;
    std::unordered_set<std::string> previous_files_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, argv[1]);
        watcher.start();

        std::thread io_thread([&io]() { io.run(); });

        std::cout << "Watching directory: " << argv[1] << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        io.stop();
        io_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}