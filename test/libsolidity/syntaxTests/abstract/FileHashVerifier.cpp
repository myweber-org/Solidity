#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filepath);
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

bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string computedHash = calculateSHA256(filepath);
        std::string lowerExpected;
        std::string lowerComputed;

        lowerExpected.reserve(expectedHash.size());
        lowerComputed.reserve(computedHash.size());

        std::transform(expectedHash.begin(), expectedHash.end(), std::back_inserter(lowerExpected), ::tolower);
        std::transform(computedHash.begin(), computedHash.end(), std::back_inserter(lowerComputed), ::tolower);

        return lowerComputed == lowerExpected;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
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
        std::cout << "SUCCESS: File hash matches the expected value." << std::endl;
        return 0;
    } else {
        std::cout << "FAILURE: File hash does NOT match the expected value." << std::endl;
        return 1;
    }
}