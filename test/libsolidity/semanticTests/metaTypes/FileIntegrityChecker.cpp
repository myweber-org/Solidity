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

bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string computedHash = computeSHA256(filepath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filepath> [expected_hash]" << std::endl;
        std::cout << "If only filepath is provided, computes and prints SHA-256 hash." << std::endl;
        std::cout << "If expected_hash is provided, verifies file integrity." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    
    if (argc == 2) {
        try {
            std::string hash = computeSHA256(filepath);
            std::cout << "SHA-256 hash: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        if (verifyFileIntegrity(filepath, expectedHash)) {
            std::cout << "Integrity check PASSED: file matches expected hash." << std::endl;
        } else {
            std::cout << "Integrity check FAILED: file does not match expected hash." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid number of arguments." << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

class FileIntegrityChecker {
public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX shaContext;
        SHA256_Init(&shaContext);

        std::vector<char> buffer(4096);
        while (file.read(buffer.data(), buffer.size())) {
            SHA256_Update(&shaContext, buffer.data(), file.gcount());
        }
        SHA256_Update(&shaContext, buffer.data(), file.gcount());

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &shaContext);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    static bool verifyIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            return actualHash == expectedHash;
        } catch (const std::exception& e) {
            std::cerr << "Verification failed: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    if (FileIntegrityChecker::verifyIntegrity(filepath, expectedHash)) {
        std::cout << "File integrity verified successfully." << std::endl;
        return 0;
    } else {
        std::cout << "File integrity check failed!" << std::endl;
        return 1;
    }
}