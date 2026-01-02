
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

class DataIntegrityVerifier {
public:
    static uint32_t calculateCRC32(const std::vector<uint8_t>& data) {
        uint32_t crc = 0xFFFFFFFF;
        for (uint8_t byte : data) {
            crc ^= byte;
            for (int i = 0; i < 8; i++) {
                uint32_t mask = -(crc & 1);
                crc = (crc >> 1) ^ (0xEDB88320 & mask);
            }
        }
        return ~crc;
    }

    static bool verifyData(const std::vector<uint8_t>& data, uint32_t expectedChecksum) {
        return calculateCRC32(data) == expectedChecksum;
    }
};

int main() {
    std::string testData = "Sample data for integrity verification";
    std::vector<uint8_t> data(testData.begin(), testData.end());

    uint32_t checksum = DataIntegrityVerifier::calculateCRC32(data);
    std::cout << "CRC32 Checksum: 0x" << std::hex << checksum << std::dec << std::endl;

    bool isValid = DataIntegrityVerifier::verifyData(data, checksum);
    std::cout << "Data integrity check: " << (isValid ? "PASSED" : "FAILED") << std::endl;

    std::vector<uint8_t> corruptedData = data;
    if (!corruptedData.empty()) {
        corruptedData[0] ^= 0xFF;
        bool isCorruptedValid = DataIntegrityVerifier::verifyData(corruptedData, checksum);
        std::cout << "Corrupted data check: " << (isCorruptedValid ? "PASSED" : "FAILED") << std::endl;
    }

    return 0;
}