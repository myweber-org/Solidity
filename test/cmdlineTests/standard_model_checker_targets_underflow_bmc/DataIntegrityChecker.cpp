
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <iomanip>

uint32_t generateCRC32(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 0;
    }

    const uint32_t polynomial = 0xEDB88320;
    uint32_t crc = 0xFFFFFFFF;

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)) || file.gcount()) {
        size_t bytesRead = file.gcount();
        for (size_t i = 0; i < bytesRead; ++i) {
            crc ^= static_cast<uint8_t>(buffer[i]);
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ polynomial;
                } else {
                    crc >>= 1;
                }
            }
        }
    }

    file.close();
    return crc ^ 0xFFFFFFFF;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    uint32_t checksum = generateCRC32(filename);

    if (checksum != 0) {
        std::cout << "CRC32 checksum for " << filename << ": 0x"
                  << std::hex << std::setw(8) << std::setfill('0') << checksum
                  << std::dec << std::endl;
    }

    return 0;
}