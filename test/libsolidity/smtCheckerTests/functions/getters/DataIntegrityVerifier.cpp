
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

class DataIntegrityVerifier {
public:
    static std::string calculateSHA256(const std::vector<unsigned char>& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.data(), data.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    static bool verifyIntegrity(const std::vector<unsigned char>& data, const std::string& expectedHash) {
        std::string calculatedHash = calculateSHA256(data);
        return calculatedHash == expectedHash;
    }
};

int main() {
    std::vector<unsigned char> sampleData = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    
    std::string hash = DataIntegrityVerifier::calculateSHA256(sampleData);
    std::cout << "SHA-256 Hash: " << hash << std::endl;
    
    bool isValid = DataIntegrityVerifier::verifyIntegrity(sampleData, hash);
    std::cout << "Data integrity check: " << (isValid ? "PASSED" : "FAILED") << std::endl;
    
    return 0;
}