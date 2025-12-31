#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class FileHandler {
public:
    static std::vector<std::string> readLines(const std::string& filename) {
        std::vector<std::string> lines;
        std::ifstream file(filename);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        } else {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }
        return lines;
    }

    static bool writeLines(const std::string& filename, const std::vector<std::string>& lines) {
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const auto& line : lines) {
                file << line << std::endl;
            }
            file.close();
            return true;
        } else {
            std::cerr << "Unable to write to file: " << filename << std::endl;
            return false;
        }
    }

    static void displayFileContent(const std::string& filename) {
        auto content = readLines(filename);
        std::cout << "Content of " << filename << ":" << std::endl;
        for (const auto& line : content) {
            std::cout << line << std::endl;
        }
    }
};

int main() {
    std::string testFile = "test_data.txt";
    std::vector<std::string> data = {"First line", "Second line", "Third line"};

    if (FileHandler::writeLines(testFile, data)) {
        std::cout << "File written successfully." << std::endl;
    }

    FileHandler::displayFileContent(testFile);

    return 0;
}