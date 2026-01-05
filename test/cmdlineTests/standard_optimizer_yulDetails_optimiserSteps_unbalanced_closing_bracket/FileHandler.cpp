#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
private:
    std::string filePath;

public:
    explicit FileHandler(const std::string& path) : filePath(path) {}

    std::string readContent() {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for reading: " + filePath);
        }

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }

        file.close();
        return content;
    }

    void writeContent(const std::string& content) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }

        file << content;
        file.close();
    }

    void appendContent(const std::string& content) {
        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for appending: " + filePath);
        }

        file << content;
        file.close();
    }

    static bool fileExists(const std::string& path) {
        std::ifstream file(path);
        return file.good();
    }
};