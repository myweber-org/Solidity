
#include <iostream>
#include <fstream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <memory>

class FileEncryptor {
private:
    static const size_t KEY_SIZE = 32;
    static const size_t IV_SIZE = 16;
    static const size_t SALT_SIZE = 16;
    static const int ITERATIONS = 100000;

    std::vector<unsigned char> deriveKey(const std::string& password, 
                                         const unsigned char* salt) {
        std::vector<unsigned char> key(KEY_SIZE);
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                         salt, SALT_SIZE,
                         ITERATIONS, EVP_sha256(),
                         KEY_SIZE, key.data());
        return key;
    }

public:
    bool encryptFile(const std::string& inputPath,
                     const std::string& outputPath,
                     const std::string& password) {
        
        std::ifstream inputFile(inputPath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Cannot open input file: " << inputPath << std::endl;
            return false;
        }

        std::ofstream outputFile(outputPath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Cannot create output file: " << outputPath << std::endl;
            return false;
        }

        unsigned char salt[SALT_SIZE];
        unsigned char iv[IV_SIZE];
        RAND_bytes(salt, SALT_SIZE);
        RAND_bytes(iv, IV_SIZE);

        outputFile.write(reinterpret_cast<char*>(salt), SALT_SIZE);
        outputFile.write(reinterpret_cast<char*>(iv), IV_SIZE);

        auto key = deriveKey(password, salt);

        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> 
            ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
        
        if (!EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr,
                               key.data(), iv)) {
            std::cerr << "Encryption initialization failed" << std::endl;
            return false;
        }

        const size_t BUFFER_SIZE = 4096;
        std::vector<unsigned char> inBuffer(BUFFER_SIZE);
        std::vector<unsigned char> outBuffer(BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH);
        
        int bytesRead, outLength;
        while (inputFile.read(reinterpret_cast<char*>(inBuffer.data()), 
                             BUFFER_SIZE)) {
            bytesRead = static_cast<int>(inputFile.gcount());
            
            if (!EVP_EncryptUpdate(ctx.get(), outBuffer.data(), &outLength,
                                  inBuffer.data(), bytesRead)) {
                std::cerr << "Encryption update failed" << std::endl;
                return false;
            }
            outputFile.write(reinterpret_cast<char*>(outBuffer.data()), outLength);
        }

        if (!EVP_EncryptFinal_ex(ctx.get(), outBuffer.data(), &outLength)) {
            std::cerr << "Encryption finalization failed" << std::endl;
            return false;
        }
        outputFile.write(reinterpret_cast<char*>(outBuffer.data()), outLength);

        OPENSSL_cleanse(key.data(), key.size());
        return true;
    }

    bool decryptFile(const std::string& inputPath,
                     const std::string& outputPath,
                     const std::string& password) {
        
        std::ifstream inputFile(inputPath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Cannot open input file: " << inputPath << std::endl;
            return false;
        }

        std::ofstream outputFile(outputPath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Cannot create output file: " << outputPath << std::endl;
            return false;
        }

        unsigned char salt[SALT_SIZE];
        unsigned char iv[IV_SIZE];
        
        inputFile.read(reinterpret_cast<char*>(salt), SALT_SIZE);
        inputFile.read(reinterpret_cast<char*>(iv), IV_SIZE);

        auto key = deriveKey(password, salt);

        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> 
            ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
        
        if (!EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr,
                               key.data(), iv)) {
            std::cerr << "Decryption initialization failed" << std::endl;
            return false;
        }

        const size_t BUFFER_SIZE = 4096 + EVP_MAX_BLOCK_LENGTH;
        std::vector<unsigned char> inBuffer(BUFFER_SIZE);
        std::vector<unsigned char> outBuffer(BUFFER_SIZE);
        
        int bytesRead, outLength;
        while (inputFile.read(reinterpret_cast<char*>(inBuffer.data()), 
                             BUFFER_SIZE)) {
            bytesRead = static_cast<int>(inputFile.gcount());
            
            if (!EVP_DecryptUpdate(ctx.get(), outBuffer.data(), &outLength,
                                  inBuffer.data(), bytesRead)) {
                std::cerr << "Decryption update failed" << std::endl;
                return false;
            }
            outputFile.write(reinterpret_cast<char*>(outBuffer.data()), outLength);
        }

        if (!EVP_DecryptFinal_ex(ctx.get(), outBuffer.data(), &outLength)) {
            std::cerr << "Decryption finalization failed - wrong password?" << std::endl;
            return false;
        }
        outputFile.write(reinterpret_cast<char*>(outBuffer.data()), outLength);

        OPENSSL_cleanse(key.data(), key.size());
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] 
                  << " <encrypt|decrypt> <input> <output> <password>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFile = argv[2];
    std::string outputFile = argv[3];
    std::string password = argv[4];

    FileEncryptor encryptor;
    bool success = false;

    if (mode == "encrypt") {
        success = encryptor.encryptFile(inputFile, outputFile, password);
        if (success) {
            std::cout << "File encrypted successfully: " << outputFile << std::endl;
        }
    } else if (mode == "decrypt") {
        success = encryptor.decryptFile(inputFile, outputFile, password);
        if (success) {
            std::cout << "File decrypted successfully: " << outputFile << std::endl;
        }
    } else {
        std::cerr << "Invalid mode. Use 'encrypt' or 'decrypt'" << std::endl;
        return 1;
    }

    return success ? 0 : 1;
}