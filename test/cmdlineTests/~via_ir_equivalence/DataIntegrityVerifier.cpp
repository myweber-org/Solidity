
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
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX shaContext;
        SHA256_Init(&shaContext);

        std::vector<char> buffer(4096);
        while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
            SHA256_Update(&shaContext, buffer.data(), file.gcount());
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &shaContext);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
        }

        return ss.str();
    }

    static bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            std::string normalizedExpected = normalizeHash(expectedHash);
            std::string normalizedActual = normalizeHash(actualHash);

            if (normalizedExpected == normalizedActual) {
                std::cout << "Integrity check PASSED for: " << filepath << std::endl;
                return true;
            } else {
                std::cout << "Integrity check FAILED for: " << filepath << std::endl;
                std::cout << "Expected: " << normalizedExpected << std::endl;
                std::cout << "Actual:   " << normalizedActual << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Verification error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    static std::string normalizeHash(const std::string& hash) {
        std::string result;
        for (char c : hash) {
            result += std::tolower(static_cast<unsigned char>(c));
        }
        return result;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        std::cout << "Example: " << argv[0] << " document.pdf a1b2c3..." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileHashVerifier::verifyFileIntegrity(filepath, expectedHash);
    
    return isValid ? 0 : 2;
}