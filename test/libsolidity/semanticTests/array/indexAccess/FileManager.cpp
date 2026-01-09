#include <iostream>
#include <fstream>
#include <string>

bool checkFileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool readFileContent(const std::string& filename, std::string& content) {
    if (!checkFileExists(filename)) {
        std::cerr << "Error: File '" << filename << "' does not exist." << std::endl;
        return false;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file '" << filename << "' for reading." << std::endl;
        return false;
    }

    std::string line;
    content.clear();
    while (std::getline(file, line)) {
        content += line + "\n";
    }

    file.close();
    return true;
}

int main() {
    std::string filename = "data.txt";
    std::string fileContent;

    if (readFileContent(filename, fileContent)) {
        std::cout << "File content:\n" << fileContent << std::endl;
    } else {
        std::cout << "Failed to read file." << std::endl;
    }

    return 0;
}