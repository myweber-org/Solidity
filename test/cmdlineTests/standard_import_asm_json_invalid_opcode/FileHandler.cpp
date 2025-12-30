
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
public:
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        
        file.close();
        return content;
    }
    
    static void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to create file: " + filename);
        }
        
        file << content;
        file.close();
    }
    
    static bool fileExists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }
};