
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

class FileWatcher {
public:
    explicit FileWatcher(const fs::path& filePath) : m_filePath(filePath) {
        if (fs::exists(m_filePath)) {
            m_lastWriteTime = fs::last_write_time(m_filePath);
        }
    }

    bool hasChanged() {
        if (!fs::exists(m_filePath)) {
            return false;
        }

        auto currentWriteTime = fs::last_write_time(m_filePath);
        if (currentWriteTime != m_lastWriteTime) {
            m_lastWriteTime = currentWriteTime;
            return true;
        }
        return false;
    }

    void startWatching(int intervalSeconds = 1) {
        std::cout << "Starting to watch file: " << m_filePath << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            if (hasChanged()) {
                std::cout << "File modified: " << m_filePath << std::endl;
            }
        }
    }

private:
    fs::path m_filePath;
    fs::file_time_type m_lastWriteTime;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    FileWatcher watcher(argv[1]);
    watcher.startWatching();

    return 0;
}