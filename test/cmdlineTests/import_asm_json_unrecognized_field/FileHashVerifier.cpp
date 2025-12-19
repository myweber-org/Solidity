
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <vector>
#include <string>

std::string computeSHA256(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::vector<char> buffer(4096);
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

bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string computedHash = computeSHA256(filepath);
        std::string expectedLower = expectedHash;
        std::transform(expectedLower.begin(), expectedLower.end(), expectedLower.begin(), ::tolower);
        std::transform(computedHash.begin(), computedHash.end(), computedHash.begin(), ::tolower);
        return computedHash == expectedLower;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filepath> [expected_hash]" << std::endl;
        std::cout << "  If only filepath provided, computes and prints SHA256 hash." << std::endl;
        std::cout << "  If expected_hash provided, verifies file against it." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    
    if (argc == 2) {
        try {
            std::string hash = computeSHA256(filepath);
            std::cout << "SHA256(" << filepath << ") = " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        bool isValid = verifyFileHash(filepath, expectedHash);
        if (isValid) {
            std::cout << "Hash verification SUCCESS: file matches expected hash." << std::endl;
            return 0;
        } else {
            std::cout << "Hash verification FAILED: file does not match expected hash." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Too many arguments." << std::endl;
        return 1;
    }

    return 0;
}