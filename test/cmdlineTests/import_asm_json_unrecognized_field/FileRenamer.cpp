
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix, const std::string& extension)
        : m_directory(directory), m_prefix(prefix), m_extension(extension) {}

    bool validateDirectory() const {
        return fs::exists(m_directory) && fs::is_directory(m_directory);
    }

    std::vector<fs::path> getFiles() const {
        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(m_directory)) {
            if (fs::is_regular_file(entry.path())) {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    void renameFiles() {
        if (!validateDirectory()) {
            std::cerr << "Error: Invalid directory path." << std::endl;
            return;
        }

        std::vector<fs::path> files = getFiles();
        if (files.empty()) {
            std::cout << "No files found in directory." << std::endl;
            return;
        }

        int counter = 1;
        for (const auto& file : files) {
            std::string newName = m_prefix + "_" + std::to_string(counter) + m_extension;
            fs::path newPath = m_directory / newName;

            try {
                fs::rename(file, newPath);
                std::cout << "Renamed: " << file.filename() << " -> " << newName << std::endl;
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << file.filename() << ": " << e.what() << std::endl;
            }
        }

        std::cout << "Renaming completed. " << (counter - 1) << " files processed." << std::endl;
    }

private:
    std::string m_directory;
    std::string m_prefix;
    std::string m_extension;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> <extension>" << std::endl;
        std::cout << "Example: " << argv[0] << " ./photos image .jpg" << std::endl;
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    std::string extension = argv[3];

    if (extension[0] != '.') {
        extension = "." + extension;
    }

    FileRenamer renamer(directory, prefix, extension);
    renamer.renameFiles();

    return 0;
}