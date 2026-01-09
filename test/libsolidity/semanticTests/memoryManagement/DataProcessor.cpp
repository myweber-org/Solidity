
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class DataProcessor {
public:
    static std::vector<double> normalizeData(const std::vector<double>& rawData) {
        if (rawData.empty()) {
            throw std::invalid_argument("Input data cannot be empty");
        }

        std::vector<double> normalized = rawData;
        double minVal = *std::min_element(normalized.begin(), normalized.end());
        double maxVal = *std::max_element(normalized.begin(), normalized.end());

        if (std::abs(maxVal - minVal) < 1e-10) {
            std::fill(normalized.begin(), normalized.end(), 0.5);
            return normalized;
        }

        for (auto& value : normalized) {
            value = (value - minVal) / (maxVal - minVal);
        }

        return normalized;
    }

    static bool validateSensorData(const std::vector<double>& data, double minRange, double maxRange) {
        if (minRange >= maxRange) {
            throw std::invalid_argument("Invalid range specification");
        }

        for (const auto& value : data) {
            if (value < minRange || value > maxRange || std::isnan(value)) {
                return false;
            }
        }
        return true;
    }

    static double calculateMovingAverage(const std::vector<double>& data, size_t windowSize) {
        if (data.empty() || windowSize == 0 || windowSize > data.size()) {
            throw std::invalid_argument("Invalid parameters for moving average");
        }

        double sum = 0.0;
        for (size_t i = 0; i < windowSize; ++i) {
            sum += data[i];
        }
        return sum / windowSize;
    }
};