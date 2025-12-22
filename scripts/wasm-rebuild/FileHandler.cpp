#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

class FileHandler {
private:
    std::string filename;

public:
    explicit FileHandler(const std::string& name) : filename(name) {}

    std::string readContent() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for reading: " + filename);
        }

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }

        file.close();
        return content;
    }

    void writeContent(const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        file << content;
        file.close();
    }

    void appendContent(const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for appending: " + filename);
        }

        file << content;
        file.close();
    }

    std::string getFilename() const {
        return filename;
    }
};

int main() {
    try {
        FileHandler handler("test_output.txt");
        handler.writeContent("Initial line.\n");
        handler.appendContent("Appended line.\n");

        std::string content = handler.readContent();
        std::cout << "File content:\n" << content;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}