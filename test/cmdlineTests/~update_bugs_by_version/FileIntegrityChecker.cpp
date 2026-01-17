#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    SHA256_CTX shaContext;
    SHA256_Init(&shaContext);

    std::vector<char> buffer(8192);
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
        SHA256_Update(&shaContext, buffer.data(), file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &shaContext);

    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result << std::setw(2) << static_cast<int>(hash[i]);
    }
    return result.str();
}

bool verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash) {
    try {
        std::string computedHash = calculateSHA256(filePath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file> [expected_hash]" << std::endl;
        std::cout << "If only file is provided, computes its SHA-256 hash." << std::endl;
        std::cout << "If expected_hash is provided, verifies file integrity." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];

    if (argc == 2) {
        try {
            std::string hash = calculateSHA256(filePath);
            std::cout << "SHA-256: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        if (verifyFileIntegrity(filePath, expectedHash)) {
            std::cout << "Integrity check PASSED." << std::endl;
            return 0;
        } else {
            std::cout << "Integrity check FAILED." << std::endl;
            return 1;
        }
    }

    return 0;
}