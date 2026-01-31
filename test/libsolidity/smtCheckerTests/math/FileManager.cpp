#include <iostream>
#include <fstream>
#include <string>
#include <system_error>

class FileManager {
public:
    static bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
            return false;
        }
        file << content;
        if (file.fail()) {
            std::cerr << "Error: Failed to write content to file " << filename << "." << std::endl;
            file.close();
            return false;
        }
        file.close();
        return true;
    }

    static std::string readFromFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        if (file.fail() && !file.eof()) {
            std::cerr << "Error: Failed to read content from file " << filename << "." << std::endl;
            file.close();
            return "";
        }
        file.close();
        return content;
    }

    static bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::out | std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for appending." << std::endl;
            return false;
        }
        file << content;
        if (file.fail()) {
            std::cerr << "Error: Failed to append content to file " << filename << "." << std::endl;
            file.close();
            return false;
        }
        file.close();
        return true;
    }
};