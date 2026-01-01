
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
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

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string computedHash = calculateSHA256(filepath);
        std::string expectedLower = expectedHash;
        std::transform(expectedLower.begin(), expectedLower.end(), expectedLower.begin(), ::tolower);
        std::transform(computedHash.begin(), computedHash.end(), computedHash.begin(), ::tolower);
        
        return computedHash == expectedLower;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <command> <file> [expected_hash]\n"
              << "Commands:\n"
              << "  compute    Compute SHA256 hash of file\n"
              << "  verify     Verify file against expected hash\n"
              << "Example:\n"
              << "  " << programName << " compute document.pdf\n"
              << "  " << programName << " verify archive.zip \"a1b2c3...\"" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    std::string filepath = argv[2];

    if (command == "compute") {
        try {
            std::string hash = calculateSHA256(filepath);
            std::cout << "SHA256: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "verify") {
        if (argc < 4) {
            std::cerr << "Error: Expected hash required for verification" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        std::string expectedHash = argv[3];
        bool isValid = verifyFileHash(filepath, expectedHash);
        std::cout << "Verification: " << (isValid ? "PASS" : "FAIL") << std::endl;
        return isValid ? 0 : 1;
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}