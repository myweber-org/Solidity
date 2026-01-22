
#include <iostream>
#include <fstream>
#include <string>

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

int main() {
    std::string testFile = "test.txt";
    
    if (fileExists(testFile)) {
        std::cout << "File '" << testFile << "' exists." << std::endl;
    } else {
        std::cout << "File '" << testFile << "' does not exist." << std::endl;
    }
    
    return 0;
}