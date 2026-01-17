
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
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
        std::string actualHash = computeSHA256(filepath);
        std::string expectedLower = expectedHash;
        std::transform(expectedLower.begin(), expectedLower.end(), expectedLower.begin(), ::tolower);
        std::transform(actualHash.begin(), actualHash.end(), actualHash.begin(), ::tolower);
        
        return actualHash == expectedLower;
    } catch (const std::exception& e) {
        std::cerr << "Verification error: " << e.what() << std::endl;
        return false;
    }
}

void printUsage(const std::string& programName) {
    std::cout << "Usage:\n"
              << "  " << programName << " compute <filepath>\n"
              << "  " << programName << " verify <filepath> <expected_hash>\n";
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
            std::string hash = computeSHA256(filepath);
            std::cout << "SHA256 hash of " << filepath << ":\n" << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "verify") {
        if (argc < 4) {
            std::cerr << "Error: Expected hash not provided for verification.\n";
            printUsage(argv[0]);
            return 1;
        }
        std::string expectedHash = argv[3];
        bool isValid = verifyFileHash(filepath, expectedHash);
        if (isValid) {
            std::cout << "Verification PASSED: File hash matches expected value.\n";
            return 0;
        } else {
            std::cout << "Verification FAILED: File hash does not match expected value.\n";
            return 1;
        }
    } else {
        std::cerr << "Error: Unknown command '" << command << "'.\n";
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}