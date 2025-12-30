#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
private:
    std::string filename;

public:
    explicit FileHandler(const std::string& name) : filename(name) {}

    std::string readContent() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for reading: " + filename);
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
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        file << content;
        file.close();
    }

    void appendContent(const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for appending: " + filename);
        }

        file << content;
        file.close();
    }

    bool fileExists() const {
        std::ifstream file(filename);
        return file.good();
    }
};