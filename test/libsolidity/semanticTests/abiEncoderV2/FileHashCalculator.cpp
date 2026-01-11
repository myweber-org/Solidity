#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class FileHashCalculator {
public:
    static std::string calculateSHA256(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filePath);
        }

        SHA256_CTX shaContext;
        SHA256_Init(&shaContext);

        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            SHA256_Update(&shaContext, buffer, file.gcount());
        }
        SHA256_Update(&shaContext, buffer, file.gcount());

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &shaContext);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        return ss.str();
    }

    static bool verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash) {
        std::string calculatedHash = calculateSHA256(filePath);
        return calculatedHash == expectedHash;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        std::string filePath = argv[1];
        std::string hash = FileHashCalculator::calculateSHA256(filePath);
        std::cout << "SHA-256 hash of " << filePath << ":\n" << hash << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}