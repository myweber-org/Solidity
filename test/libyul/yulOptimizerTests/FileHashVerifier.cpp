
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

std::string computeSHA256(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        SHA256_Update(&sha256, buffer.data(), file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string actualHash = computeSHA256(filepath);
        for (char& c : actualHash) c = std::tolower(c);
        std::string expectedLower = expectedHash;
        for (char& c : expectedLower) c = std::tolower(c);
        return actualHash == expectedLower;
    } catch (const std::exception& e) {
        std::cerr << "Verification error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    if (verifyFileHash(filepath, expectedHash)) {
        std::cout << "SUCCESS: File hash matches expected value." << std::endl;
        return 0;
    } else {
        std::cout << "FAILURE: File hash does not match expected value." << std::endl;
        return 2;
    }
}