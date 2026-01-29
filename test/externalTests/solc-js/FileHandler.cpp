#include <iostream>
#include <fstream>
#include <string>
#include <system_error>

class FileHandler {
public:
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        
        if (!file.is_open()) {
            throw std::system_error(errno, std::system_category(), "Failed to open file: " + filename);
        }
        
        std::string content;
        file.seekg(0, std::ios::end);
        content.reserve(file.tellg());
        file.seekg(0, std::ios::beg);
        
        content.assign((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
        
        if (file.bad()) {
            throw std::runtime_error("Error occurred while reading file: " + filename);
        }
        
        return content;
    }
    
    static void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::binary);
        
        if (!file.is_open()) {
            throw std::system_error(errno, std::system_category(), "Failed to create file: " + filename);
        }
        
        file.write(content.data(), content.size());
        
        if (file.bad()) {
            throw std::runtime_error("Error occurred while writing to file: " + filename);
        }
    }
};

int main() {
    try {
        std::string testContent = "This is a test file content.\nLine 2 of content.\n";
        FileHandler::writeFile("test_output.txt", testContent);
        
        std::string readContent = FileHandler::readFile("test_output.txt");
        std::cout << "File content read successfully:\n" << readContent << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}