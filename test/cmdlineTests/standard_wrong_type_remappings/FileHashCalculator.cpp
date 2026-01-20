#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <openssl/evp.h>
#include <openssl/err.h>

class FileHashCalculator {
public:
    static std::string calculateSHA256(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        if (mdctx == nullptr) {
            throw std::runtime_error("Failed to create EVP_MD_CTX");
        }

        if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to initialize digest");
        }

        const size_t bufferSize = 4096;
        char buffer[bufferSize];
        while (file.good()) {
            file.read(buffer, bufferSize);
            std::streamsize bytesRead = file.gcount();
            if (bytesRead > 0) {
                if (EVP_DigestUpdate(mdctx, buffer, bytesRead) != 1) {
                    EVP_MD_CTX_free(mdctx);
                    throw std::runtime_error("Failed to update digest");
                }
            }
        }

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLength = 0;
        if (EVP_DigestFinal_ex(mdctx, hash, &hashLength) != 1) {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Failed to finalize digest");
        }

        EVP_MD_CTX_free(mdctx);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < hashLength; ++i) {
            ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
        }
        return ss.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    try {
        std::string hash = FileHashCalculator::calculateSHA256(argv[1]);
        std::cout << "SHA-256 hash: " << hash << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}