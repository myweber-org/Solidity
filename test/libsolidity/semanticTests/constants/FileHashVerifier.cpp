
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <vector>
#include <string>

class FileHashVerifier {
public:
    static std::string computeSHA256(const std::string& filepath) {
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

    static bool verifyHash(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string computedHash = computeSHA256(filepath);
            return computedHash == expectedHash;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_hash>" << std::endl;
        std::cout << "Example: " << argv[0] << " document.pdf a1b2c3..." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    if (FileHashVerifier::verifyHash(filepath, expectedHash)) {
        std::cout << "Hash verification SUCCESS: File integrity confirmed." << std::endl;
        return 0;
    } else {
        std::cout << "Hash verification FAILED: File may be corrupted or tampered." << std::endl;
        return 1;
    }
}
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
        return actualHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        std::cout << "Or: " << argv[0] << " <filepath> (to compute hash only)" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    if (expectedHash.empty()) {
        try {
            std::string hash = computeSHA256(filepath);
            std::cout << "SHA256: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else {
        bool isValid = verifyFileHash(filepath, expectedHash);
        if (isValid) {
            std::cout << "File hash verification PASSED." << std::endl;
            return 0;
        } else {
            std::cout << "File hash verification FAILED." << std::endl;
            return 1;
        }
    }

    return 0;
}