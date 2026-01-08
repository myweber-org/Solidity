
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>

class DataProcessor {
public:
    static std::vector<double> normalizeData(const std::vector<double>& input) {
        if (input.empty()) {
            throw std::invalid_argument("Input vector cannot be empty");
        }

        auto minMax = std::minmax_element(input.begin(), input.end());
        double minVal = *minMax.first;
        double maxVal = *minMax.second;
        double range = maxVal - minVal;

        if (range == 0.0) {
            return std::vector<double>(input.size(), 0.5);
        }

        std::vector<double> normalized;
        normalized.reserve(input.size());
        for (double value : input) {
            normalized.push_back((value - minVal) / range);
        }
        return normalized;
    }

    static std::vector<double> standardizeData(const std::vector<double>& input) {
        if (input.size() < 2) {
            throw std::invalid_argument("Input vector must contain at least 2 elements");
        }

        double sum = 0.0;
        for (double value : input) {
            sum += value;
        }
        double mean = sum / input.size();

        double variance = 0.0;
        for (double value : input) {
            variance += (value - mean) * (value - mean);
        }
        double stdDev = std::sqrt(variance / (input.size() - 1));

        if (stdDev == 0.0) {
            return std::vector<double>(input.size(), 0.0);
        }

        std::vector<double> standardized;
        standardized.reserve(input.size());
        for (double value : input) {
            standardized.push_back((value - mean) / stdDev);
        }
        return standardized;
    }

    static bool validateData(const std::vector<double>& input) {
        if (input.empty()) {
            return false;
        }

        for (double value : input) {
            if (std::isnan(value) || std::isinf(value)) {
                return false;
            }
        }
        return true;
    }
};