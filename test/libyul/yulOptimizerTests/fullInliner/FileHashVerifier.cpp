#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

std::string computeSHA256(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath);
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
    try {
        std::string computedHash = computeSHA256(filePath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error during verification: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file_path> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    std::string expectedHash = argv[2];

    if (verifyFileIntegrity(filePath, expectedHash)) {
        std::cout << "File integrity verified. Hashes match." << std::endl;
        return 0;
    } else {
        std::cout << "File integrity check failed. Hashes do not match." << std::endl;
        return 1;
    }
}