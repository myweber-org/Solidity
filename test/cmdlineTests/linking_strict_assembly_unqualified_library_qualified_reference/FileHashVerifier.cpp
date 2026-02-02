
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
        std::streamsize fileSize = getFileSize(file);

        while (file) {
            file.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = file.gcount();
            
            if (bytesRead > 0) {
                SHA256_Update(&sha256, buffer.data(), bytesRead);
                totalBytes += bytesRead;
                displayProgress(totalBytes, fileSize);
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }

    static bool verifyFileHash(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            std::cout << "\nCalculated hash: " << actualHash << std::endl;
            std::cout << "Expected hash:   " << expectedHash << std::endl;
            
            bool match = (actualHash == expectedHash);
            std::cout << "Verification: " << (match ? "PASS" : "FAIL") << std::endl;
            return match;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    static std::streamsize getFileSize(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        return size;
    }

    static void displayProgress(std::streamsize current, std::streamsize total) {
        if (total <= 0) return;
        
        int percentage = static_cast<int>((current * 100) / total);
        std::cout << "\rProgress: " << percentage << "% [";
        
        int barWidth = 50;
        int pos = static_cast<int>(barWidth * percentage / 100.0);
        
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        
        std::cout << "] " << current << "/" << total << " bytes";
        std::cout.flush();
        
        if (current >= total) {
            std::cout << std::endl;
        }
    }

    static std::string bytesToHex(const unsigned char* bytes, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<int>(bytes[i]);
        }
        
        return ss.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        std::cout << "Example: " << argv[0] << " important_document.pdf a1b2c3..." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::string expectedHash = argv[2];

    bool isValid = FileHashVerifier::verifyFileHash(filepath, expectedHash);
    
    return isValid ? 0 : 1;
}