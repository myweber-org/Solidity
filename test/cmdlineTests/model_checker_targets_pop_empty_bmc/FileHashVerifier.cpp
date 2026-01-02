
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

class FileHashVerifier {
public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        const size_t bufferSize = 65536;
        std::vector<char> buffer(bufferSize);
        std::streamsize totalBytes = 0;
        std::streamsize fileSize = getFileSize(filepath);

        while (file.read(buffer.data(), bufferSize) || file.gcount() > 0) {
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

private:
    static std::streamsize getFileSize(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) return 0;
        return file.tellg();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        std::cout << "Example: " << argv[0] << " data.bin a1b2c3..." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileHashVerifier::verifyFileIntegrity(filepath, expectedHash);
    return isValid ? 0 : 1;
}