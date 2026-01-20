#include <iostream>
#include <fstream>
#include <string>
#include <system_error>

class FileHandler {
public:
    static bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    static std::string readFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file for reading: " << filename << std::endl;
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        return content;
    }

    static bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error opening file for appending: " << filename << std::endl;
            return false;
        }
        file << content;
        file.close();
        return true;
    }
};

int main() {
    const std::string testFile = "test_data.txt";
    const std::string testContent = "This is a test file content.\nSecond line of text.\n";

    if (FileHandler::writeToFile(testFile, testContent)) {
        std::cout << "File written successfully." << std::endl;
    }

    std::string readContent = FileHandler::readFromFile(testFile);
    if (!readContent.empty()) {
        std::cout << "File content:\n" << readContent << std::endl;
    }

    if (FileHandler::appendToFile(testFile, "Appended line.\n")) {
        std::cout << "Content appended successfully." << std::endl;
    }

    readContent = FileHandler::readFromFile(testFile);
    if (!readContent.empty()) {
        std::cout << "Updated file content:\n" << readContent << std::endl;
    }

    return 0;
}