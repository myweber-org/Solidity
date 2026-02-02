
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

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::vector<char> buffer(8192);
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
        SHA256_Update(&sha256, buffer.data(), file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return oss.str();
}

bool verifyFileHash(const std::string& filePath, const std::string& expectedHash) {
    try {
        std::string computedHash = computeSHA256(filePath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <file_path> <expected_hash>\n";
        std::cout << "Or: " << argv[0] << " <file_path> (to compute hash only)\n";
        return 1;
    }

    std::string filePath = argv[1];
    
    if (argc == 2) {
        try {
            std::string hash = computeSHA256(filePath);
            std::cout << "SHA256 hash of " << filePath << ":\n" << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::string expectedHash = argv[2];
        if (verifyFileHash(filePath, expectedHash)) {
            std::cout << "Hash verification SUCCESSFUL\n";
            return 0;
        } else {
            std::cout << "Hash verification FAILED\n";
            return 1;
        }
    }
    
    return 0;
}