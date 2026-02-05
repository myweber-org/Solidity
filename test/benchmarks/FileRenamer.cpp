
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void renameFilesInDirectory(const fs::path& directory, const std::string& baseName) {
    std::vector<fs::directory_entry> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            files.push_back(entry);
        }
    }
    
    std::sort(files.begin(), files.end(), 
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.path().filename().string() < b.path().filename().string();
              });
    
    int counter = 1;
    for (const auto& file : files) {
        fs::path oldPath = file.path();
        std::string extension = oldPath.extension().string();
        
        std::string newFileName = baseName + "_" + std::to_string(counter) + extension;
        fs::path newPath = directory / newFileName;
        
        try {
            fs::rename(oldPath, newPath);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newFileName << std::endl;
            counter++;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Total files renamed: " << counter - 1 << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <base_name>" << std::endl;
        return 1;
    }
    
    fs::path targetDirectory(argv[1]);
    std::string baseName(argv[2]);
    
    if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }
    
    renameFilesInDirectory(targetDirectory, baseName);
    
    return 0;
}
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

bool renameFileWithTimestamp(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        std::cerr << "Error: File does not exist." << std::endl;
        return false;
    }

    if (!fs::is_regular_file(filePath)) {
        std::cerr << "Error: Path is not a regular file." << std::endl;
        return false;
    }

    std::string timestamp = getCurrentTimestamp();
    fs::path parentDir = filePath.parent_path();
    std::string extension = filePath.extension().string();
    std::string stem = filePath.stem().string();

    fs::path newFilePath = parentDir / (timestamp + "_" + stem + extension);

    if (fs::exists(newFilePath)) {
        std::cerr << "Error: Target file already exists." << std::endl;
        return false;
    }

    try {
        fs::rename(filePath, newFilePath);
        std::cout << "Renamed: " << filePath.filename() << " -> " << newFilePath.filename() << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    fs::path filePath(argv[1]);
    return renameFileWithTimestamp(filePath) ? 0 : 1;
}
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix)
        : m_directory(directory), m_prefix(prefix) {}

    bool renameFiles() {
        std::vector<fs::path> files;
        
        try {
            for (const auto& entry : fs::directory_iterator(m_directory)) {
                if (fs::is_regular_file(entry.status())) {
                    files.push_back(entry.path());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing directory: " << e.what() << std::endl;
            return false;
        }

        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return true;
        }

        std::sort(files.begin(), files.end());

        int counter = 1;
        for (const auto& oldPath : files) {
            std::string extension = oldPath.extension().string();
            std::string newFilename = m_prefix + "_" + std::to_string(counter) + extension;
            fs::path newPath = oldPath.parent_path() / newFilename;

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newFilename << std::endl;
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << std::endl;
            }
        }

        return true;
    }

private:
    std::string m_directory;
    std::string m_prefix;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix>" << std::endl;
        return 1;
    }

    FileRenamer renamer(argv[1], argv[2]);
    if (!renamer.renameFiles()) {
        return 1;
    }

    return 0;
}