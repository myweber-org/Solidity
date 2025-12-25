
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <openssl/sha.h>

class FileIntegrityChecker {
public:
    static std::string computeSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256Context;
        SHA256_Init(&sha256Context);

        std::vector<char> buffer(4096);
        while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
            SHA256_Update(&sha256Context, buffer.data(), file.gcount());
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256Context);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::setw(2) << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    static bool verifyChecksum(const std::string& filepath, const std::string& expectedHash) {
        std::string computedHash = computeSHA256(filepath);
        return computedHash == expectedHash;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filepath> [expected_sha256]" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    try {
        std::string hash = FileIntegrityChecker::computeSHA256(filepath);
        std::cout << "SHA256: " << hash << std::endl;

        if (argc == 3) {
            std::string expectedHash = argv[2];
            bool isValid = FileIntegrityChecker::verifyChecksum(filepath, expectedHash);
            std::cout << "Verification: " << (isValid ? "PASS" : "FAIL") << std::endl;
            return isValid ? 0 : 2;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}