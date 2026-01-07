
#include <iostream>
#include <fstream>
#include <string>
#include <system_error>

class FileHandler {
public:
    static bool writeToFile(const std::string& filename, const std::string& content) {
        std::ofstream outFile(filename, std::ios::out);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
            return false;
        }
        outFile << content;
        outFile.close();
        return true;
    }

    static std::string readFromFile(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::in);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(inFile)),
                             std::istreambuf_iterator<char>());
        inFile.close();
        return content;
    }

    static bool appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream outFile(filename, std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for appending." << std::endl;
            return false;
        }
        outFile << content;
        outFile.close();
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