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

int main() {
    try {
        const std::string testFilename = "test_output.txt";
        const std::string testContent = "Hello, this is a test file.\nCreated by FileHandler utility.\n";
        
        std::cout << "Writing to file: " << testFilename << std::endl;
        FileHandler::writeFile(testFilename, testContent);
        
        std::cout << "Reading from file: " << testFilename << std::endl;
        std::string readContent = FileHandler::readFile(testFilename);
        std::cout << "File content:\n" << readContent;
        
        std::cout << "File exists check: " 
                  << (FileHandler::fileExists(testFilename) ? "Yes" : "No") 
                  << std::endl;
                  
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}