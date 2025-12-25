
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

class FileHashVerifier {
private:
    static const size_t BUFFER_SIZE = 4096;

    static std::string bytesToHex(const unsigned char* bytes, size_t length) {
        std::stringstream ss;
        for(size_t i = 0; i < length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
        }
        return ss.str();
    }

public:
    static std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if(!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::vector<char> buffer(BUFFER_SIZE);
        size_t totalBytes = 0;
        std::streamsize bytesRead;

        while(file.read(buffer.data(), BUFFER_SIZE) || (bytesRead = file.gcount()) > 0) {
            SHA256_Update(&sha256, buffer.data(), bytesRead);
            totalBytes += bytesRead;
            
            if(totalBytes % (10 * 1024 * 1024) == 0) {
                std::cout << "Processed " << (totalBytes / (1024 * 1024)) << " MB\r" << std::flush;
            }
        }

        file.close();

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        std::cout << "\nTotal processed: " << totalBytes << " bytes\n";
        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }

    static bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string calculatedHash = calculateSHA256(filepath);
            std::cout << "Calculated hash: " << calculatedHash << std::endl;
            std::cout << "Expected hash:   " << expectedHash << std::endl;
            
            bool match = (calculatedHash == expectedHash);
            std::cout << "Verification: " << (match ? "PASSED" : "FAILED") << std::endl;
            return match;
        } catch(const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if(argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>\n";
        std::cout << "Example: " << argv[0] << " important_document.pdf a1b2c3...\n";
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileHashVerifier::verifyFileHash(filepath, expectedHash);
    return isValid ? 0 : 1;
}