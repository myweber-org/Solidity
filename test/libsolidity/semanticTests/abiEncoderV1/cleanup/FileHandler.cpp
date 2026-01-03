#include <iostream>
#include <fstream>
#include <string>
#include <system_error>

class FileHandler {
public:
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::system_error(errno, std::system_category(), "Failed to open file: " + filename);
        }
        
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        
        if (file.bad()) {
            throw std::system_error(errno, std::system_category(), "Error reading file: " + filename);
        }
        
        return content;
    }
    
    static void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::system_error(errno, std::system_category(), "Failed to create file: " + filename);
        }
        
        file << content;
        
        if (file.bad()) {
            throw std::system_error(errno, std::system_category(), "Error writing to file: " + filename);
        }
    }
    
    static void appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            throw std::system_error(errno, std::system_category(), "Failed to open file for appending: " + filename);
        }
        
        file << content;
        
        if (file.bad()) {
            throw std::system_error(errno, std::system_category(), "Error appending to file: " + filename);
        }
    }
};

int main() {
    try {
        // Test the file handler
        std::string testContent = "This is a test file.\nIt contains multiple lines.\n";
        
        FileHandler::writeFile("test_output.txt", testContent);
        std::cout << "File written successfully.\n";
        
        FileHandler::appendToFile("test_output.txt", "This line was appended.\n");
        std::cout << "Content appended successfully.\n";
        
        std::string readContent = FileHandler::readFile("test_output.txt");
        std::cout << "File content:\n" << readContent;
        
    } catch (const std::system_error& e) {
        std::cerr << "Error: " << e.what() << " (code: " << e.code() << ")\n";
        return 1;
    }
    
    return 0;
}