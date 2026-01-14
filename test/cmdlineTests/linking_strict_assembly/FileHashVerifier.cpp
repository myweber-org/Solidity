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

    SHA256_CTX shaContext;
    SHA256_Init(&shaContext);

    std::vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size())) {
        SHA256_Update(&shaContext, buffer.data(), file.gcount());
    }
    SHA256_Update(&shaContext, buffer.data(), file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &shaContext);

    std::ostringstream result;
    result << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result << std::setw(2) << static_cast<int>(hash[i]);
    }

    return result.str();
}

bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
    try {
        std::string actualHash = calculateSHA256(filepath);
        return actualHash == expectedHash;
    } catch (const std::exception& e) {
        std::cerr << "Verification failed: " << e.what() << std::endl;
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

    if (verifyFileIntegrity(filepath, expectedHash)) {
        std::cout << "File integrity verified successfully." << std::endl;
        return 0;
    } else {
        std::cout << "File integrity check failed. Hash mismatch or file error." << std::endl;
        return 1;
    }
}
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>
#include <filesystem>

class FileHashVerifier {
public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::vector<char> buffer(8192);
        std::streamsize totalBytes = 0;
        std::streamsize fileSize = std::filesystem::file_size(filepath);

        while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
            SHA256_Update(&sha256, buffer.data(), file.gcount());
            totalBytes += file.gcount();

            if (fileSize > 0) {
                int progress = static_cast<int>((totalBytes * 100) / fileSize);
                std::cout << "\rProcessing: " << progress << "% complete" << std::flush;
            }
        }

        std::cout << std::endl;

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        return ss.str();
    }

    static bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            std::cout << "Calculated hash: " << actualHash << std::endl;
            std::cout << "Expected hash:   " << expectedHash << std::endl;

            bool match = (actualHash == expectedHash);
            std::cout << "Verification: " << (match ? "PASSED" : "FAILED") << std::endl;
            return match;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
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
    return isValid ? 0 : 1;
}