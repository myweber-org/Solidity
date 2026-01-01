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
            throw std::runtime_error("Failed to create or open file: " + filename);
        }

        file << content;
        file.close();
    }

    static void appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for appending: " + filename);
        }

        file << content;
        file.close();
    }
};

int main() {
    try {
        FileHandler::writeFile("test.txt", "Hello, World!\n");
        FileHandler::appendToFile("test.txt", "This is appended text.\n");
        
        std::string fileContent = FileHandler::readFile("test.txt");
        std::cout << "File content:\n" << fileContent << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}