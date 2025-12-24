
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <zlib.h>
#include <vector>
#include <mutex>

namespace fs = std::filesystem;

class FileLogger {
private:
    std::string base_path;
    size_t max_file_size;
    int max_backup_files;
    std::mutex log_mutex;
    std::ofstream current_stream;
    std::string current_filename;
    size_t current_size;

    std::string generate_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        return ss.str();
    }

    void rotate_if_needed() {
        if (current_size >= max_file_size) {
            current_stream.close();
            std::string new_name = current_filename + "_" + generate_timestamp() + ".log";
            fs::rename(current_filename, new_name);
            compress_file(new_name);
            cleanup_old_files();
            open_new_file();
        }
    }

    void compress_file(const std::string& filename) {
        std::ifstream in_file(filename, std::ios::binary);
        if (!in_file) return;

        std::vector<char> buffer(
            (std::istreambuf_iterator<char>(in_file)),
            std::istreambuf_iterator<char>()
        );
        in_file.close();

        uLongf compressed_size = compressBound(buffer.size());
        std::vector<Bytef> compressed(compressed_size);

        if (compress(compressed.data(), &compressed_size,
                    reinterpret_cast<Bytef*>(buffer.data()), buffer.size()) == Z_OK) {
            std::ofstream out_file(filename + ".gz", std::ios::binary);
            out_file.write(reinterpret_cast<char*>(compressed.data()), compressed_size);
            out_file.close();
            fs::remove(filename);
        }
    }

    void cleanup_old_files() {
        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(base_path)) {
            if (entry.path().extension() == ".gz" &&
                entry.path().string().find(base_path) != std::string::npos) {
                files.push_back(entry);
            }
        }

        std::sort(files.begin(), files.end(),
                 [](const fs::directory_entry& a, const fs::directory_entry& b) {
                     return fs::last_write_time(a) > fs::last_write_time(b);
                 });

        for (size_t i = max_backup_files; i < files.size(); ++i) {
            fs::remove(files[i].path());
        }
    }

    void open_new_file() {
        current_filename = base_path + "/app_" + generate_timestamp() + ".log";
        current_stream.open(current_filename, std::ios::app);
        current_size = 0;
    }

public:
    FileLogger(const std::string& path = "./logs",
               size_t max_size = 1048576 * 10, // 10MB
               int max_backups = 5)
        : base_path(path), max_file_size(max_size),
          max_backup_files(max_backups), current_size(0) {
        if (!fs::exists(base_path)) {
            fs::create_directories(base_path);
        }
        open_new_file();
    }

    ~FileLogger() {
        if (current_stream.is_open()) {
            current_stream.close();
        }
    }

    void log(const std::string& message, const std::string& level = "INFO") {
        std::lock_guard<std::mutex> lock(log_mutex);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream log_entry;
        log_entry << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                  << "." << std::setfill('0') << std::setw(3) << ms.count()
                  << " [" << level << "] " << message << "\n";

        std::string entry_str = log_entry.str();
        current_stream << entry_str;
        current_stream.flush();
        current_size += entry_str.size();

        rotate_if_needed();
    }

    void info(const std::string& message) {
        log(message, "INFO");
    }

    void warning(const std::string& message) {
        log(message, "WARN");
    }

    void error(const std::string& message) {
        log(message, "ERROR");
    }

    void debug(const std::string& message) {
        log(message, "DEBUG");
    }
};