
#include <iostream>
#include <fstream>
#include <string>

class FileManager {
public:
    static bool fileExists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }

    static void checkAndReport(const std::string& filename) {
        if (fileExists(filename)) {
            std::cout << "File '" << filename << "' exists and is accessible." << std::endl;
        } else {
            std::cerr << "Error: File '" << filename << "' does not exist or cannot be accessed." << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    FileManager::checkAndReport(filename);
    return 0;
}