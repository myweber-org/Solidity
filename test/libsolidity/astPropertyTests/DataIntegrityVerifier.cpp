
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

class DataVerifier {
public:
    static std::string calculateSHA256(const std::string& filepath) {
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
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        return ss.str();
    }

    static bool verifyIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            return actualHash == expectedHash;
        } catch (const std::exception& e) {
            std::cerr << "Verification error: " << e.what() << std::endl;
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

    bool isValid = DataVerifier::verifyIntegrity(filepath, expectedHash);

    if (isValid) {
        std::cout << "Data integrity verified successfully." << std::endl;
        return 0;
    } else {
        std::cout << "Data integrity check failed." << std::endl;
        return 1;
    }
}