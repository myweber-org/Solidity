#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

class DataProcessor {
public:
    static std::vector<double> normalizeNumericData(const std::vector<double>& input) {
        if (input.empty()) {
            return {};
        }

        double minVal = *std::min_element(input.begin(), input.end());
        double maxVal = *std::max_element(input.begin(), input.end());
        double range = maxVal - minVal;

        std::vector<double> normalized;
        normalized.reserve(input.size());

        if (range == 0.0) {
            for (const auto& val : input) {
                normalized.push_back(0.5);
            }
        } else {
            for (const auto& val : input) {
                normalized.push_back((val - minVal) / range);
            }
        }

        return normalized;
    }

    static bool validateAndCleanString(std::string& str) {
        bool isValid = true;

        str.erase(std::remove_if(str.begin(), str.end(),
            [&isValid](unsigned char c) {
                if (std::isprint(c) == 0) {
                    isValid = false;
                    return true;
                }
                return false;
            }), str.end());

        if (str.length() > 100) {
            str = str.substr(0, 100);
            isValid = false;
        }

        return isValid;
    }

    static void processUserInput(const std::vector<double>& numbers, std::string& text) {
        std::cout << "Processing numeric data..." << std::endl;
        auto normalized = normalizeNumericData(numbers);

        if (!normalized.empty()) {
            std::cout << "Normalized values: ";
            for (const auto& val : normalized) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "Validating and cleaning text..." << std::endl;
        bool textValid = validateAndCleanString(text);

        if (textValid) {
            std::cout << "Text is valid. Cleaned text: " << text << std::endl;
        } else {
            std::cout << "Text was modified during cleaning. Result: " << text << std::endl;
        }
    }
};

int main() {
    std::vector<double> sampleData = {10.5, 2.3, 45.6, 8.9, 33.1};
    std::string sampleText = "Hello, World! This is a test string with some \x01 non-printable characters.";

    DataProcessor::processUserInput(sampleData, sampleText);

    return 0;
}