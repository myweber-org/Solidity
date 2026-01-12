
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

class FileManager {
public:
    static bool readAllLines(const std::string& filepath, std::vector<std::string>& lines) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file " << filepath << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        file.close();
        return true;
    }

    static bool writeAllLines(const std::string& filepath, const std::vector<std::string>& lines) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to create file " << filepath << std::endl;
            return false;
        }

        for (const auto& line : lines) {
            file << line << std::endl;
        }

        file.close();
        return true;
    }

    static bool copyFile(const std::string& source, const std::string& destination) {
        std::error_code ec;
        fs::copy(source, destination, fs::copy_options::overwrite_existing, ec);
        
        if (ec) {
            std::cerr << "Error copying file: " << ec.message() << std::endl;
            return false;
        }
        return true;
    }

    static bool fileExists(const std::string& filepath) {
        return fs::exists(filepath);
    }

    static uintmax_t getFileSize(const std::string& filepath) {
        if (!fileExists(filepath)) {
            return 0;
        }
        return fs::file_size(filepath);
    }
};