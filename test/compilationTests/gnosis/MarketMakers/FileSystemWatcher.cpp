#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

void watchDirectory(const fs::path& directory) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: " << directory << " is not a valid directory." << std::endl;
        return;
    }

    std::cout << "Watching directory: " << directory << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    auto lastWriteTime = fs::last_write_time(directory);
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        try {
            auto currentWriteTime = fs::last_write_time(directory);
            
            if (currentWriteTime != lastWriteTime) {
                lastWriteTime = currentWriteTime;
                std::cout << "Directory modified at: " 
                          << std::chrono::system_clock::to_time_t(
                                std::chrono::file_clock::to_sys(currentWriteTime)
                             ) << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    fs::path watchPath = (argc > 1) ? argv[1] : fs::current_path();
    
    try {
        watchDirectory(watchPath);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}