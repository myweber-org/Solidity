#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    SHA256_CTX sha256Context;
    SHA256_Init(&sha256Context);

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
        SHA256_Update(&sha256Context, buffer.data(), file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256Context);

    std::ostringstream resultStream;
    resultStream << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        resultStream << std::setw(2) << static_cast<int>(hash[i]);
    }
    return resultStream.str();
}

bool verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash) {
    try {
        std::string computedHash = calculateSHA256(filePath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error during verification: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file_path> [expected_hash]" << std::endl;
        std::cout << "If only file_path is provided, the SHA-256 hash is computed and displayed." << std::endl;
        std::cout << "If expected_hash is provided, the program verifies the file's integrity." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];

    if (argc == 2) {
        try {
            std::string hash = calculateSHA256(filePath);
            std::cout << "SHA-256 hash of '" << filePath << "':" << std::endl;
            std::cout << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        if (verifyFileIntegrity(filePath, expectedHash)) {
            std::cout << "Integrity check PASSED. File hash matches the expected value." << std::endl;
        } else {
            std::cout << "Integrity check FAILED. File hash does not match the expected value." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid number of arguments." << std::endl;
        return 1;
    }

    return 0;
}