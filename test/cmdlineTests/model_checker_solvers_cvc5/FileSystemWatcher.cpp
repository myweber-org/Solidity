#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>

namespace fs = std::filesystem;

class FileSystemWatcher {
public:
    FileSystemWatcher(const std::string& path) : watchPath(path) {
        if (fs::exists(watchPath) && fs::is_directory(watchPath)) {
            populateFileMap();
        }
    }

    void startWatching(int intervalSeconds = 1) {
        std::cout << "Watching directory: " << watchPath << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            checkForChanges();
        }
    }

private:
    std::string watchPath;
    std::unordered_map<std::string, fs::file_time_type> fileMap;

    void populateFileMap() {
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
                auto currentWriteTime = fs::last_write_time(entry.path());

                if (fileMap.find(filePath) == fileMap.end()) {
                    std::cout << "[NEW] " << filePath << std::endl;
                    fileMap[filePath] = currentWriteTime;
                } else if (fileMap[filePath] != currentWriteTime) {
                    std::cout << "[MODIFIED] " << filePath << std::endl;
                    fileMap[filePath] = currentWriteTime;
                }
            }
        }

        std::vector<std::string> filesToRemove;
        for (const auto& [filePath, writeTime] : fileMap) {
            if (!fs::exists(filePath)) {
                std::cout << "[DELETED] " << filePath << std::endl;
                filesToRemove.push_back(filePath);
            }
        }

        for (const auto& filePath : filesToRemove) {
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