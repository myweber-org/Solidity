#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <string>
#include <vector>

class FileHashCalculator {
public:
    static std::string calculateSHA256(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        SHA256_CTX shaContext;
        SHA256_Init(&shaContext);

        std::vector<char> buffer(4096);
        while (file.good()) {
            file.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = file.gcount();
            if (bytesRead > 0) {
                SHA256_Update(&shaContext, buffer.data(), bytesRead);
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &shaContext);

        std::ostringstream resultStream;
        resultStream << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            resultStream << std::setw(2) << static_cast<int>(hash[i]);
        }

        return resultStream.str();
    }

    static void printHash(const std::string& filePath) {
        try {
            std::string hashValue = calculateSHA256(filePath);
            std::cout << "SHA-256 hash of '" << filePath << "':\n";
            std::cout << hashValue << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    FileHashCalculator::printHash(filePath);

    return 0;
}