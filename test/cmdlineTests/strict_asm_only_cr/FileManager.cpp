#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileManager {
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

    static bool fileExists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }
};

int main() {
    try {
        const std::string testFilename = "test_data.txt";
        const std::string testContent = "Hello, World!\nThis is a test file.\nCreated by FileManager.";

        if (FileManager::fileExists(testFilename)) {
            std::cout << "File already exists. Reading content...\n";
            std::string existingContent = FileManager::readFile(testFilename);
            std::cout << "Existing content:\n" << existingContent << "\n";
        } else {
            std::cout << "File does not exist. Creating new file...\n";
            FileManager::writeFile(testFilename, testContent);
            std::cout << "File created successfully.\n";

            std::string readContent = FileManager::readFile(testFilename);
            std::cout << "File content:\n" << readContent;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}