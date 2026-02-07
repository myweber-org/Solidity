#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
private:
    std::string filename;

public:
    explicit FileHandler(const std::string& name) : filename(name) {}

    bool writeContent(const std::string& content) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing.\n";
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    std::string readContent() {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            throw std::runtime_error("Error: Could not open file " + filename + " for reading.");
        }

        std::string content;
        std::string line;
        while (std::getline(inFile, line)) {
            content += line + "\n";
        }
        inFile.close();
        return content;
    }

    bool appendContent(const std::string& content) {
        std::ofstream outFile(filename, std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for appending.\n";
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    std::string getFilename() const {
        return filename;
    }
};