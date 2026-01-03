
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
public:
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        
        std::string content;
        file.seekg(0, std::ios::end);
        content.reserve(file.tellg());
        file.seekg(0, std::ios::beg);
        
        content.assign((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
        
        return content;
    }
    
    static void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to create file: " + filename);
        }
        
        file.write(content.c_str(), content.size());
        if (!file.good()) {
            throw std::runtime_error("Failed to write to file: " + filename);
        }
    }
    
    static bool fileExists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }
};