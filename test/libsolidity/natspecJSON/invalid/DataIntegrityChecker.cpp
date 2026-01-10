
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

class CRC32 {
public:
    CRC32() {
        generateTable();
    }

    uint32_t compute(const std::vector<uint8_t>& data) {
        uint32_t crc = 0xFFFFFFFF;
        for (uint8_t byte : data) {
            uint8_t index = (crc ^ byte) & 0xFF;
            crc = (crc >> 8) ^ table[index];
        }
        return crc ^ 0xFFFFFFFF;
    }

    uint32_t compute(const std::string& str) {
        std::vector<uint8_t> data(str.begin(), str.end());
        return compute(data);
    }

    bool verify(const std::vector<uint8_t>& data, uint32_t expected_crc) {
        return compute(data) == expected_crc;
    }

    bool verify(const std::string& str, uint32_t expected_crc) {
        return compute(str) == expected_crc;
    }

private:
    uint32_t table[256];

    void generateTable() {
        const uint32_t polynomial = 0xEDB88320;
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ polynomial;
                } else {
                    crc >>= 1;
                }
            }
            table[i] = crc;
        }
    }
};

int main() {
    CRC32 crc_calculator;

    std::string test_data = "Hello, this is a test string for CRC32 validation.";
    uint32_t checksum = crc_calculator.compute(test_data);

    std::cout << "Data: " << test_data << std::endl;
    std::cout << "CRC32 Checksum: 0x" << std::hex << checksum << std::dec << std::endl;

    bool is_valid = crc_calculator.verify(test_data, checksum);
    std::cout << "Verification: " << (is_valid ? "PASS" : "FAIL") << std::endl;

    std::string corrupted_data = test_data + "x";
    bool is_corrupted_valid = crc_calculator.verify(corrupted_data, checksum);
    std::cout << "Corrupted data verification: " << (is_corrupted_valid ? "PASS" : "FAIL") << std::endl;

    return 0;
}