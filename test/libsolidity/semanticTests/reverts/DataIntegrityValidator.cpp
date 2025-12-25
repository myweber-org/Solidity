
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

class DataIntegrityValidator {
public:
    static std::string calculateSHA256(const std::string& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.c_str(), data.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    static bool verifyDataIntegrity(const std::string& data, const std::string& expectedHash) {
        std::string calculatedHash = calculateSHA256(data);
        return calculatedHash == expectedHash;
    }

    static void validateAndReport(const std::string& data, const std::string& expectedHash) {
        std::cout << "Data length: " << data.length() << " bytes" << std::endl;
        std::cout << "Expected hash: " << expectedHash << std::endl;
        
        std::string calculatedHash = calculateSHA256(data);
        std::cout << "Calculated hash: " << calculatedHash << std::endl;
        
        if(verifyDataIntegrity(data, expectedHash)) {
            std::cout << "✓ Data integrity verified successfully" << std::endl;
        } else {
            std::cout << "✗ Data integrity check failed!" << std::endl;
            std::cout << "Hash mismatch detected" << std::endl;
        }
    }
};

int main() {
    std::string testData = "This is a critical data payload that requires integrity verification.";
    std::string storedHash = "a1b2c3d4e5f678901234567890123456789012345678901234567890123456";
    
    DataIntegrityValidator::validateAndReport(testData, storedHash);
    
    std::string correctData = "This is the correct data that should match the hash.";
    std::string correctHash = DataIntegrityValidator::calculateSHA256(correctData);
    
    std::cout << "\nCorrect data validation:" << std::endl;
    DataIntegrityValidator::validateAndReport(correctData, correctHash);
    
    return 0;
}