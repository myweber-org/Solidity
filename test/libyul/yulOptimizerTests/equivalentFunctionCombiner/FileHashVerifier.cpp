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
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::vector<char> buffer(65536);
        std::streamsize totalBytes = 0;
        std::streamsize fileSize = getFileSize(file);

        while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
            SHA256_Update(&sha256, buffer.data(), file.gcount());
            totalBytes += file.gcount();
            
            if (fileSize > 0) {
                int progress = static_cast<int>((totalBytes * 100) / fileSize);
                std::cout << "\rProcessing: " << progress << "% complete" << std::flush;
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        std::cout << "\nFile processed successfully. Total bytes: " << totalBytes << std::endl;

        return bytesToHex(hash, SHA256_DIGEST_LENGTH);
    }

    static bool verifyFileIntegrity(const std::string& filepath, const std::string& expectedHash) {
        try {
            std::string actualHash = calculateSHA256(filepath);
            std::cout << "Expected: " << expectedHash << std::endl;
            std::cout << "Actual:   " << actualHash << std::endl;
            
            bool match = (actualHash == expectedHash);
            std::cout << "Verification: " << (match ? "PASSED" : "FAILED") << std::endl;
            return match;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    static std::streamsize getFileSize(std::ifstream& file) {
        std::streampos current = file.tellg();
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(current, std::ios::beg);
        return size;
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
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filepath> [expected_hash]" << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  " << argv[0] << " document.pdf" << std::endl;
        std::cout << "  " << argv[0] << " archive.zip e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    
    if (argc == 3) {
        std::string expectedHash = argv[2];
        FileHashVerifier::verifyFileIntegrity(filepath, expectedHash);
    } else {
        std::string hash = FileHashVerifier::calculateSHA256(filepath);
        std::cout << "SHA-256: " << hash << std::endl;
    }

    return 0;
}