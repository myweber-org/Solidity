#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const fs::path& path) : watchPath(path) {
        if (!fs::exists(watchPath) || !fs::is_directory(watchPath)) {
            throw std::runtime_error("Path does not exist or is not a directory");
        }
        snapshotFiles();
    }

    void startWatching(int intervalSeconds = 1) {
        std::cout << "Watching directory: " << watchPath << std::endl;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            checkForChanges();
        }
    }

private:
    fs::path watchPath;
    std::unordered_map<std::string, fs::file_time_type> fileMap;

    void snapshotFiles() {
        fileMap.clear();
        for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
            if (fs::is_regular_file(entry.path())) {
                fileMap[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    }

    void checkForChanges() {
        for (const auto& entry : fs::recursive_directory_iterator(watchPath)) {
            if (fs::is_regular_file(entry.path())) {
                std::string filePath = entry.path().string();
                auto currentTime = fs::last_write_time(entry.path());

                if (fileMap.find(filePath) == fileMap.end()) {
                    std::cout << "New file detected: " << filePath << std::endl;
                    fileMap[filePath] = currentTime;
                } else if (fileMap[filePath] != currentTime) {
                    std::cout << "File modified: " << filePath << std::endl;
                    fileMap[filePath] = currentTime;
                }
            }
        }

        std::vector<std::string> toRemove;
        for (const auto& [filePath, _] : fileMap) {
            if (!fs::exists(filePath)) {
                std::cout << "File deleted: " << filePath << std::endl;
                toRemove.push_back(filePath);
            }
        }

        for (const auto& filePath : toRemove) {
            fileMap.erase(filePath);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    try {
        FileSystemWatcher watcher(argv[1]);
        watcher.startWatching();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}