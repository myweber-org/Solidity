
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size())) {
        SHA256_Update(&sha256, buffer.data(), file.gcount());
    }
    SHA256_Update(&sha256, buffer.data(), file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string actualHash = calculateSHA256(filepath);
        return actualHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

void printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  FileIntegrityChecker <filepath> [expected_hash]\n";
    std::cout << "Options:\n";
    std::cout << "  filepath: Path to the file to check\n";
    std::cout << "  expected_hash: Optional SHA256 hash for verification\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string filepath = argv[1];
    
    try {
        std::string hash = calculateSHA256(filepath);
        std::cout << "SHA256: " << hash << std::endl;
        
        if (argc == 3) {
            std::string expectedHash = argv[2];
            if (verifyFileIntegrity(filepath, expectedHash)) {
                std::cout << "Integrity check: PASSED" << std::endl;
                return 0;
            } else {
                std::cout << "Integrity check: FAILED" << std::endl;
                return 1;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}