
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    SHA256_CTX shaContext;
    SHA256_Init(&shaContext);

    std::vector<char> buffer(4096);
    while (file.good()) {
        file.read(buffer.data(), buffer.size());
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
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <file_path> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    std::string expectedHash = argv[2];

    if (verifyFileIntegrity(filePath, expectedHash)) {
        std::cout << "File integrity verified. Hashes match." << std::endl;
        return 0;
    } else {
        std::cout << "WARNING: File integrity check failed. Hashes do not match!" << std::endl;
        return 2;
    }
}