
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

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

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::setw(2) << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    static bool verifyFile(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = computeSHA256(filepath);
            return actualHash == expectedHash;
        } catch (const std::exception& e) {
            std::cerr << "Verification error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filepath> [expected_hash]" << std::endl;
        std::cout << "If only filepath provided, computes SHA256 hash." << std::endl;
        std::cout << "If expected_hash provided, verifies file integrity." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    
    if (argc == 2) {
        try {
            std::string hash = FileHashVerifier::computeSHA256(filepath);
            std::cout << "SHA256: " << hash << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    } else if (argc == 3) {
        std::string expectedHash = argv[2];
        bool isValid = FileHashVerifier::verifyFile(filepath, expectedHash);
        std::cout << "Verification: " << (isValid ? "PASS" : "FAIL") << std::endl;
        return isValid ? 0 : 1;
    }

    return 0;
}