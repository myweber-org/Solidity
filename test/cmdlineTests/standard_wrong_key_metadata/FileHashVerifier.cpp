
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

        std::vector<char> buffer(8192);
        std::streamsize totalBytes = 0;
        
        while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
            SHA256_Update(&sha256, buffer.data(), file.gcount());
            totalBytes += file.gcount();
            
            if (totalBytes % (10 * 1024 * 1024) == 0) {
                std::cout << "Processed " << (totalBytes / (1024 * 1024)) << " MB...\r" << std::flush;
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        std::cout << "\nTotal processed: " << (totalBytes / (1024 * 1024)) << " MB" << std::endl;
        return ss.str();
    }

    static bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            std::cout << "Expected: " << expectedHash << std::endl;
            std::cout << "Actual:   " << actualHash << std::endl;
            
            return (actualHash == expectedHash);
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

    bool isValid = FileHashVerifier::verifyFileHash(filepath, expectedHash);
    
    if (isValid) {
        std::cout << "✓ File hash verification PASSED" << std::endl;
        return 0;
    } else {
        std::cout << "✗ File hash verification FAILED" << std::endl;
        return 1;
    }
}