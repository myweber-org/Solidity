
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

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

    static bool writeToFile(const std::string& filepath, const std::string& content) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to create or write to file " << filepath << std::endl;
            return false;
        }

        file << content;
        file.close();
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