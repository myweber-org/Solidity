
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <zlib.h>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

class FileLogger {
public:
    explicit FileLogger(const std::string& base_path, size_t max_size = 1048576, int max_files = 10)
        : base_path_(base_path), max_file_size_(max_size), max_backup_files_(max_files) {
        open_current_log();
    }

    void log(const std::string& message) {
        if (!current_stream_ || !current_stream_->is_open()) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        localtime_r(&time, &tm_buf);

        char time_str[64];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

        *current_stream_ << "[" << time_str << "] " << message << std::endl;

        if (current_stream_->tellp() > max_file_size_) {
            rotate_log();
        }
    }

private:
    void open_current_log() {
        current_path_ = base_path_ + ".log";
        current_stream_ = std::make_unique<std::ofstream>(current_path_, std::ios::app);
    }

    void rotate_log() {
        if (!current_stream_) return;

        current_stream_->close();

        if (fs::exists(current_path_)) {
            compress_and_archive();
        }

        cleanup_old_files();
        open_current_log();
    }

    void compress_and_archive() {
        std::ifstream input(current_path_, std::ios::binary);
        if (!input) return;

        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        input.close();

        std::string archive_name = generate_archive_name();
        std::string compressed_path = base_path_ + "." + archive_name + ".gz";

        gzFile gz = gzopen(compressed_path.c_str(), "wb");
        if (!gz) return;

        gzwrite(gz, buffer.data(), buffer.size());
        gzclose(gz);

        fs::remove(current_path_);
    }

    std::string generate_archive_name() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        localtime_r(&time, &tm_buf);

        char time_str[32];
        std::strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", &tm_buf);
        return std::string(time_str);
    }

    void cleanup_old_files() {
        std::vector<fs::directory_entry> archives;
        for (const auto& entry : fs::directory_iterator(fs::path(base_path_).parent_path())) {
            if (entry.path().string().find(base_path_ + ".") != std::string::npos &&
                entry.path().extension() == ".gz") {
                archives.push_back(entry);
            }
        }

        std::sort(archives.begin(), archives.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return fs::last_write_time(a) < fs::last_write_time(b);
                  });

        while (archives.size() > max_backup_files_) {
            fs::remove(archives.front().path());
            archives.erase(archives.begin());
        }
    }

    std::string base_path_;
    std::string current_path_;
    size_t max_file_size_;
    int max_backup_files_;
    std::unique_ptr<std::ofstream> current_stream_;
};