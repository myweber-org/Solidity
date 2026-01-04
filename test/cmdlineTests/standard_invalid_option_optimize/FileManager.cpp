
#include <iostream>
#include <fstream>
#include <string>
#include <system_error>

class FileManager {
public:
    static bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }
        
        file << content;
        
        if (file.fail()) {
            std::cerr << "Error writing to file: " << filename << std::endl;
            file.close();
            return false;
        }
        
        file.close();
        return true;
    }
    
    static std::string readFromFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::in);
        if (!file.is_open()) {
            throw std::system_error(errno, std::system_category(), 
                                   "Failed to open file: " + filename);
        }
        
        std::string content;
        std::string line;
        
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        
        if (file.bad()) {
            throw std::system_error(errno, std::system_category(),
                                   "Error reading file: " + filename);
        }
        
        file.close();
        return content;
    }
    
    static bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::out | std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error opening file for append: " << filename << std::endl;
            return false;
        }
        
        file << content;
        
        if (file.fail()) {
            std::cerr << "Error appending to file: " << filename << std::endl;
            file.close();
            return false;
        }
        
        file.close();
        return true;
    }
};