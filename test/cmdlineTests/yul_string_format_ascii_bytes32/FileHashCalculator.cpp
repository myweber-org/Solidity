
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/err.h>

std::string computeSHA256(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return "";
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        return "";
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    while (file.good()) {
        file.read(buffer, bufferSize);
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0) {
            if (EVP_DigestUpdate(mdctx, buffer, bytesRead) != 1) {
                EVP_MD_CTX_free(mdctx);
                return "";
            }
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;
    if (EVP_DigestFinal_ex(mdctx, hash, &hashLength) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hashLength; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filepath>" << std::endl;
        return 1;
    }

    std::string hash = computeSHA256(argv[1]);
    if (hash.empty()) {
        std::cerr << "Error computing hash or file not found." << std::endl;
        return 1;
    }

    std::cout << "SHA256: " << hash << std::endl;
    return 0;
}