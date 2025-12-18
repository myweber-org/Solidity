#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string base_filename;
    std::string log_directory;
    size_t max_file_size;
    int max_backup_files;
    std::ofstream current_stream;
    size_t current_size;

    std::string generate_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotate_if_needed() {
        if (current_size >= max_file_size) {
            current_stream.close();
            std::string timestamp = generate_timestamp();
            std::string new_name = log_directory + "/" + base_filename + "_" + timestamp + ".log";
            fs::rename(log_directory + "/" + base_filename + ".log", new_name);

            cleanup_old_files();
            open_current_log();
        }
    }

    void cleanup_old_files() {
        std::vector<fs::path> log_files;
        for (const auto& entry : fs::directory_iterator(log_directory)) {
            if (entry.is_regular_file() && entry.path().filename().string().find(base_filename) == 0) {
                log_files.push_back(entry.path());
            }
        }

        std::sort(log_files.begin(), log_files.end(),
                  [](const fs::path& a, const fs::path& b) {
                      return fs::last_write_time(a) > fs::last_write_time(b);
                  });

        for (size_t i = max_backup_files; i < log_files.size(); ++i) {
            fs::remove(log_files[i]);
        }
    }

    void open_current_log() {
        std::string current_path = log_directory + "/" + base_filename + ".log";
        current_stream.open(current_path, std::ios::app);
        current_size = fs::file_size(current_path);
    }

public:
    FileLogger(const std::string& filename, const std::string& dir = "logs",
               size_t max_size = 1048576, int max_backups = 10)
        : base_filename(filename), log_directory(dir), max_file_size(max_size),
          max_backup_files(max_backups), current_size(0) {
        if (!fs::exists(log_directory)) {
            fs::create_directories(log_directory);
        }
        open_current_log();
    }

    ~FileLogger() {
        if (current_stream.is_open()) {
            current_stream.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "[%Y-%m-%d %H:%M:%S] ");
        std::string log_entry = ss.str() + message + "\n";

        current_stream << log_entry;
        current_stream.flush();
        current_size += log_entry.size();

        rotate_if_needed();
    }

    void log_error(const std::string& message) {
        log("[ERROR] " + message);
    }

    void log_warning(const std::string& message) {
        log("[WARNING] " + message);
    }

    void log_info(const std::string& message) {
        log("[INFO] " + message);
    }

private:
    std::mutex log_mutex;
};