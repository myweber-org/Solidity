#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>

class FileHashCalculator {
private:
    static const size_t BUFFER_SIZE = 4096;
    
    std::string calculateSHA256(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }
        
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        
        std::vector<char> buffer(BUFFER_SIZE);
        size_t totalBytes = 0;
        
        while (file.good()) {
            file.read(buffer.data(), buffer.size());
            size_t bytesRead = file.gcount();
            
            if (bytesRead > 0) {
                SHA256_Update(&sha256, buffer.data(), bytesRead);
                totalBytes += bytesRead;
                
                if (totalBytes % (10 * 1024 * 1024) == 0) {
                    std::cout << "Processed " << (totalBytes / (1024 * 1024)) 
                              << " MB..." << std::endl;
                }
            }
        }
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
        }
        
        return ss.str();
    }
    
public:
    void verifyFileIntegrity(const std::string& filepath, 
                            const std::string& expectedHash = "") {
        try {
            std::cout << "Calculating SHA-256 hash for: " << filepath << std::endl;
            std::string calculatedHash = calculateSHA256(filepath);
            
            std::cout << "\nSHA-256 Hash: " << calculatedHash << std::endl;
            
            if (!expectedHash.empty()) {
                std::cout << "Expected Hash: " << expectedHash << std::endl;
                
                if (calculatedHash == expectedHash) {
                    std::cout << "✓ Integrity check PASSED" << std::endl;
                } else {
                    std::cout << "✗ Integrity check FAILED" << std::endl;
                }
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void compareTwoFiles(const std::string& file1, const std::string& file2) {
        try {
            std::cout << "Comparing file integrity..." << std::endl;
            std::string hash1 = calculateSHA256(file1);
            std::string hash2 = calculateSHA256(file2);
            
            std::cout << "File 1 (" << file1 << "): " << hash1 << std::endl;
            std::cout << "File 2 (" << file2 << "): " << hash2 << std::endl;
            
            if (hash1 == hash2) {
                std::cout << "✓ Files are identical" << std::endl;
            } else {
                std::cout << "✗ Files are different" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    FileHashCalculator calculator;
    
    if (argc == 2) {
        calculator.verifyFileIntegrity(argv[1]);
    } else if (argc == 3) {
        calculator.compareTwoFiles(argv[1], argv[2]);
    } else if (argc == 4 && std::string(argv[1]) == "--verify") {
        calculator.verifyFileIntegrity(argv[2], argv[3]);
    } else {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " <file>              - Calculate file hash" << std::endl;
        std::cout << "  " << argv[0] << " <file1> <file2>     - Compare two files" << std::endl;
        std::cout << "  " << argv[0] << " --verify <file> <hash> - Verify against expected hash" << std::endl;
        return 1;
    }
    
    return 0;
}