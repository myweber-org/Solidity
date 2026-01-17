
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/sha.h>

class DataVerifier {
public:
    static std::string calculateSHA256(const std::vector<unsigned char>& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.data(), data.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    static bool verifyIntegrity(const std::vector<unsigned char>& data, 
                               const std::string& expectedHash) {
        std::string calculatedHash = calculateSHA256(data);
        return calculatedHash == expectedHash;
    }
};

int main() {
    std::string testData = "Critical system configuration data";
    std::vector<unsigned char> data(testData.begin(), testData.end());
    
    std::string hash = DataVerifier::calculateSHA256(data);
    std::cout << "SHA-256 Hash: " << hash << std::endl;
    
    bool isValid = DataVerifier::verifyIntegrity(data, hash);
    std::cout << "Data integrity check: " << (isValid ? "PASSED" : "FAILED") << std::endl;
    
    return 0;
}