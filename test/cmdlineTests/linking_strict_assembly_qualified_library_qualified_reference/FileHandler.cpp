#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
private:
    std::string filePath;

public:
    explicit FileHandler(const std::string& path) : filePath(path) {}

    bool writeContent(const std::string& content) {
        std::ofstream outFile(filePath);
        if (!outFile.is_open()) {
            std::cerr << "Error: Unable to open file for writing: " << filePath << std::endl;
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    std::string readContent() {
        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            throw std::runtime_error("Unable to open file for reading: " + filePath);
        }
        std::string content((std::istreambuf_iterator<char>(inFile)),
                             std::istreambuf_iterator<char>());
        inFile.close();
        return content;
    }

    bool appendContent(const std::string& content) {
        std::ofstream outFile(filePath, std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error: Unable to open file for appending: " << filePath << std::endl;
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    void displayFileInfo() const {
        std::ifstream inFile(filePath, std::ios::binary | std::ios::ate);
        if (inFile.is_open()) {
            auto size = inFile.tellg();
            std::cout << "File: " << filePath << "\nSize: " << size << " bytes" << std::endl;
            inFile.close();
        } else {
            std::cout << "File: " << filePath << " (does not exist or cannot be accessed)" << std::endl;
        }
    }
};