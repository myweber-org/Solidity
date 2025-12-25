
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

class FileIntegrityChecker {
public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256Context;
        SHA256_Init(&sha256Context);

        std::vector<char> buffer(4096);
        while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
            SHA256_Update(&sha256Context, buffer.data(), file.gcount());
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256Context);

        std::stringstream hexStream;
        hexStream << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            hexStream << std::setw(2) << static_cast<int>(hash[i]);
        }

        return hexStream.str();
    }

    static bool verifyIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            return actualHash == expectedHash;
        } catch (const std::exception& e) {
            std::cerr << "Verification failed: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileIntegrityChecker::verifyIntegrity(filepath, expectedHash);
    
    if (isValid) {
        std::cout << "File integrity verified successfully." << std::endl;
        return 0;
    } else {
        std::cout << "File integrity check failed!" << std::endl;
        return 1;
    }
}