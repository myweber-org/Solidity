
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

bool renameFileWithTimestamp(const fs::path& filepath) {
    if (!fs::exists(filepath)) {
        std::cerr << "Error: File does not exist." << std::endl;
        return false;
    }
    
    if (!fs::is_regular_file(filepath)) {
        std::cerr << "Error: Path is not a regular file." << std::endl;
        return false;
    }
    
    std::string timestamp = getCurrentTimestamp();
    fs::path parent = filepath.parent_path();
    std::string extension = filepath.extension().string();
    std::string stem = filepath.stem().string();
    
    fs::path new_path = parent / (timestamp + "_" + stem + extension);
    
    try {
        fs::rename(filepath, new_path);
        std::cout << "Renamed: " << filepath.filename() << " -> " << new_path.filename() << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <filepath>" << std::endl;
        return 1;
    }
    
    fs::path target_file(argv[1]);
    return renameFileWithTimestamp(target_file) ? 0 : 1;
}