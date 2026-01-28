#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    explicit FileSystemWatcher(const fs::path& directory) : watch_directory(directory) {
        if (!fs::exists(watch_directory) || !fs::is_directory(watch_directory)) {
            throw std::runtime_error("Invalid directory path provided.");
        }
        populate_initial_state();
    }

    void start_watching(int interval_seconds = 2) {
        std::cout << "Watching directory: " << watch_directory << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            check_for_new_files();
        }
    }

private:
    fs::path watch_directory;
    std::unordered_set<std::string> known_files;

    void populate_initial_state() {
        known_files.clear();
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                known_files.insert(entry.path().filename().string());
            }
        }
    }

    void check_for_new_files() {
        for (const auto& entry : fs::directory_iterator(watch_directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (known_files.find(filename) == known_files.end()) {
                    std::cout << "New file detected: " << filename << std::endl;
                    known_files.insert(filename);
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.start_watching();
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
#include <unordered_map>

namespace fs = boost::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(boost::asio::io_context& io, const std::string& path)
        : timer_(io), watch_path_(path), running_(false) {
        scan_files();
    }

    void start() {
        running_ = true;
        schedule_timer();
    }

    void stop() {
        running_ = false;
        timer_.cancel();
    }

private:
    void scan_files() {
        current_files_.clear();
        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    current_files_[entry.path().string()] = fs::last_write_time(entry.path());
                }
            }
        }
    }

    void schedule_timer() {
        timer_.expires_after(std::chrono::seconds(2));
        timer_.async_wait([this](const boost::system::error_code& ec) {
            if (!ec && running_) {
                check_for_changes();
                schedule_timer();
            }
        });
    }

    void check_for_changes() {
        std::unordered_map<std::string, fs::file_time_type> new_files;
        bool changes_detected = false;

        if (fs::exists(watch_path_) && fs::is_directory(watch_path_)) {
            for (const auto& entry : fs::directory_iterator(watch_path_)) {
                if (fs::is_regular_file(entry.path())) {
                    std::string path = entry.path().string();
                    auto new_time = fs::last_write_time(entry.path());
                    new_files[path] = new_time;

                    auto it = current_files_.find(path);
                    if (it == current_files_.end()) {
                        std::cout << "File added: " << path << std::endl;
                        changes_detected = true;
                    } else if (it->second != new_time) {
                        std::cout << "File modified: " << path << std::endl;
                        changes_detected = true;
                    }
                }
            }

            for (const auto& [path, time] : current_files_) {
                if (new_files.find(path) == new_files.end()) {
                    std::cout << "File removed: " << path << std::endl;
                    changes_detected = true;
                }
            }
        }

        if (changes_detected) {
            current_files_.swap(new_files);
        }
    }

    boost::asio::steady_timer timer_;
    std::string watch_path_;
    std::unordered_map<std::string, fs::file_time_type> current_files_;
    bool running_;
};

int main() {
    try {
        boost::asio::io_context io;
        FileSystemWatcher watcher(io, ".");
        
        std::cout << "Watching current directory for file changes..." << std::endl;
        watcher.start();
        
        std::thread io_thread([&io]() { io.run(); });
        
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        watcher.stop();
        io.stop();
        io_thread.join();
        
        std::cout << "File watching stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}