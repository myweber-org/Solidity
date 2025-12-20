
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

        const size_t bufferSize = 65536;
        std::vector<char> buffer(bufferSize);
        std::streamsize totalBytes = 0;
        std::streamsize fileSize = getFileSize(file);

        while (file) {
            file.read(buffer.data(), bufferSize);
            std::streamsize bytesRead = file.gcount();
            
            if (bytesRead > 0) {
                SHA256_Update(&sha256, buffer.data(), bytesRead);
                totalBytes += bytesRead;
                displayProgress(totalBytes, fileSize);
            }
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256);

        return bytesToHexString(hash, SHA256_DIGEST_LENGTH);
    }

private:
    static std::streamsize getFileSize(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        return size;
    }

    static void displayProgress(std::streamsize current, std::streamsize total) {
        if (total == 0) return;
        
        int percentage = static_cast<int>((current * 100) / total);
        std::cout << "\rProcessing: " << percentage << "% complete" << std::flush;
        
        if (current >= total) {
            std::cout << std::endl;
        }
    }

    static std::string bytesToHexString(const unsigned char* bytes, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<int>(bytes[i]);
        }
        
        return ss.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
        return 1;
    }

    try {
        std::string filepath = argv[1];
        std::string hash = FileHashVerifier::calculateSHA256(filepath);
        
        std::cout << "SHA-256 hash: " << hash << std::endl;
        std::cout << "File verification complete." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}