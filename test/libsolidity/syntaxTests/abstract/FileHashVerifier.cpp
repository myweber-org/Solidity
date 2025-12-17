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
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        return ss.str();
    }

public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::vector<char> buffer(BUFFER_SIZE);
        size_t totalBytes = 0;
        std::streamsize bytesRead;

        while (file.read(buffer.data(), BUFFER_SIZE) || (bytesRead = file.gcount()) > 0) {
            SHA256_Update(&sha256, buffer.data(), bytesRead);
            totalBytes += bytesRead;
            
            if (totalBytes % (10 * 1024 * 1024) == 0) {
                std::cout << "\rProcessed: " << (totalBytes / (1024 * 1024)) << " MB";
                std::cout.flush();
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        std::cout << "\nTotal size: " << (totalBytes / (1024 * 1024)) << " MB\n";
        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }

    static bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::cout << "Calculating SHA-256 hash for: " << filepath << std::endl;
            std::string actualHash = calculateSHA256(filepath);
            
            std::cout << "Expected: " << expectedHash << std::endl;
            std::cout << "Actual:   " << actualHash << std::endl;
            
            bool match = (actualHash == expectedHash);
            std::cout << "Verification: " << (match ? "PASS" : "FAIL") << std::endl;
            
            return match;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>\n";
        std::cout << "Example: " << argv[0] << " document.pdf a1b2c3...\n";
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileHashVerifier::verifyFileHash(filepath, expectedHash);
    return isValid ? 0 : 1;
}