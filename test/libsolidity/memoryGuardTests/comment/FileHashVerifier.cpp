#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

class FileHashVerifier {
private:
    static const size_t BUFFER_SIZE = 4096;

    static std::string bytesToHex(const unsigned char* data, size_t length) {
        std::stringstream ss;
        for(size_t i = 0; i < length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        }
        return ss.str();
    }

public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if(!file) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::vector<char> buffer(BUFFER_SIZE);
        std::streamsize totalBytes = 0;
        std::streamsize fileSize = 0;

        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::cout << "Calculating SHA-256 for: " << filepath << std::endl;
        std::cout << "File size: " << fileSize << " bytes" << std::endl;

        while(file.read(buffer.data(), BUFFER_SIZE) || file.gcount() > 0) {
            SHA256_Update(&sha256, buffer.data(), file.gcount());
            totalBytes += file.gcount();

            int progress = static_cast<int>((totalBytes * 100) / fileSize);
            std::cout << "\rProgress: " << progress << "% (" << totalBytes << "/" << fileSize << " bytes)";
            std::cout.flush();
        }

        std::cout << std::endl;

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }

    static bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string calculatedHash = calculateSHA256(filepath);
            std::cout << "Calculated hash: " << calculatedHash << std::endl;
            std::cout << "Expected hash:   " << expectedHash << std::endl;

            bool match = (calculatedHash == expectedHash);
            if(match) {
                std::cout << "✓ File integrity verified successfully!" << std::endl;
            } else {
                std::cout << "✗ File integrity check failed!" << std::endl;
            }
            return match;
        } catch(const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if(argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        std::cout << "Example: " << argv[0] << " important_document.pdf a1b2c3..." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileHashVerifier::verifyFileIntegrity(filepath, expectedHash);
    return isValid ? 0 : 1;
}
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
        std::string actualHash = computeSHA256(filepath);
        return actualHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filepath> [expected_hash]" << std::endl;
        std::cout << "If only filepath is provided, computes SHA256 hash." << std::endl;
        std::cout << "If expected_hash is provided, verifies file integrity." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];

    if (argc == 2) {
        try {
            std::string hash = computeSHA256(filepath);
            std::cout << "SHA256: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to compute hash: " << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        if (verifyFileIntegrity(filepath, expectedHash)) {
            std::cout << "Integrity check PASSED." << std::endl;
        } else {
            std::cout << "Integrity check FAILED." << std::endl;
            return 1;
        }
    }

    return 0;
}