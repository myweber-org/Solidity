
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

class FileHashVerifier {
public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
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

    static bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string calculatedHash = calculateSHA256(filepath);
            return calculatedHash == expectedHash;
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

    bool isValid = FileHashVerifier::verifyFileIntegrity(filepath, expectedHash);

    if (isValid) {
        std::cout << "File integrity verified successfully." << std::endl;
        return 0;
    } else {
        std::cout << "File integrity check failed. Hashes do not match." << std::endl;
        return 1;
    }
}
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

    std::vector<char> buffer(8192);
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
        SHA256_Update(&sha256, buffer.data(), file.gcount());
    }

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
        std::string computedHash = calculateSHA256(filepath);
        return computedHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    if (verifyFileIntegrity(filepath, expectedHash)) {
        std::cout << "Integrity check PASSED. File hash matches." << std::endl;
        return 0;
    } else {
        std::cout << "Integrity check FAILED. File hash does not match." << std::endl;
        return 1;
    }
}