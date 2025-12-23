#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    SHA256_CTX sha256Context;
    SHA256_Init(&sha256Context);

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        SHA256_Update(&sha256Context, buffer.data(), file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256Context);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    return ss.str();
}

bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string computedHash = calculateSHA256(filepath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filepath> [expected_hash]" << std::endl;
        std::cout << "If only filepath is provided, computes and prints SHA-256 hash." << std::endl;
        std::cout << "If expected_hash is provided, verifies file integrity." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    if (argc == 2) {
        try {
            std::string hash = calculateSHA256(filepath);
            std::cout << "SHA-256 hash: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        if (verifyFileIntegrity(filepath, expectedHash)) {
            std::cout << "Integrity check PASSED." << std::endl;
        } else {
            std::cout << "Integrity check FAILED." << std::endl;
            return 1;
        }
    }

    return 0;
}