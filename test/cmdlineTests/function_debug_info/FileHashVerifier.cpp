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

        std::vector<char> buffer(65536);
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
        std::string actualHash = calculateSHA256(filepath);
        std::cout << "\nCalculated hash: " << actualHash << std::endl;
        std::cout << "Expected hash:   " << expectedHash << std::endl;
        
        bool match = (actualHash == expectedHash);
        std::cout << "Verification: " << (match ? "PASS" : "FAIL") << std::endl;
        return match;
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
        std::cout << "\rProcessing: " << percentage << "% [";
        
        int bars = percentage / 2;
        for (int i = 0; i < 50; ++i) {
            std::cout << (i < bars ? '=' : ' ');
        }
        std::cout << "] " << current << "/" << total << " bytes";
        std::cout.flush();
    }

    static std::string bytesToHex(const unsigned char* data, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        
        return ss.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <filepath> <expected_sha256_hash>" << std::endl;
        return 1;
    }

    try {
        std::string filepath = argv[1];
        std::string expectedHash = argv[2];
        
        bool isValid = FileHashVerifier::verifyFileHash(filepath, expectedHash);
        return isValid ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}