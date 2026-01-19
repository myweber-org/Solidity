#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "ERROR: Could not open file.";
    }

    SHA256_CTX sha256Context;
    SHA256_Init(&sha256Context);

    std::vector<char> buffer(8192);
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
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

bool verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash) {
    std::string computedHash = calculateSHA256(filePath);
    if (computedHash.substr(0, 5) == "ERROR") {
        std::cerr << computedHash << std::endl;
        return false;
    }
    return computedHash == expectedHash;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file_path> [expected_sha256_hash]" << std::endl;
        std::cout << "If only file path is provided, the SHA-256 hash is computed and displayed." << std::endl;
        std::cout << "If expected hash is provided, the program verifies the file's integrity." << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    if (argc == 2) {
        std::string hash = calculateSHA256(filePath);
        std::cout << "SHA-256 hash of '" << filePath << "':" << std::endl;
        std::cout << hash << std::endl;
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        if (verifyFileIntegrity(filePath, expectedHash)) {
            std::cout << "Integrity check PASSED." << std::endl;
        } else {
            std::cout << "Integrity check FAILED." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Too many arguments." << std::endl;
        return 1;
    }

    return 0;
}