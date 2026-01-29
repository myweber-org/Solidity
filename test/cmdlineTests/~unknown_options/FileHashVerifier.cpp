
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

std::string computeSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    SHA256_CTX shaContext;
    SHA256_Init(&shaContext);

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size())) {
        SHA256_Update(&shaContext, buffer.data(), file.gcount());
    }
    SHA256_Update(&shaContext, buffer.data(), file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &shaContext);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
}

bool verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash) {
    try {
        std::string computedHash = computeSHA256(filePath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file_path> [expected_hash]" << std::endl;
        std::cout << "If only file_path is provided, computes and prints SHA-256 hash." << std::endl;
        std::cout << "If expected_hash is provided, verifies file integrity." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];

    if (argc == 2) {
        try {
            std::string hash = computeSHA256(filePath);
            std::cout << "SHA-256 hash of " << filePath << ":\n" << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
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