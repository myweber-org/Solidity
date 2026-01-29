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

        if (std::fabs(range) < 1e-10) {
            return std::vector<double>(input.size(), 0.5);
        }

        std::vector<double> normalized;
        normalized.reserve(input.size());
        for (double val : input) {
            normalized.push_back((val - minVal) / range);
        }
        return normalized;
    }

    static bool validateData(const std::vector<double>& data, double lowerBound, double upperBound) {
        if (lowerBound >= upperBound) {
            throw std::invalid_argument("Lower bound must be less than upper bound");
        }

        for (double val : data) {
            if (val < lowerBound || val > upperBound) {
                return false;
            }
        }
        return true;
    }

    static double calculateMean(const std::vector<double>& data) {
        if (data.empty()) {
            throw std::invalid_argument("Data vector cannot be empty");
        }

        double sum = 0.0;
        for (double val : data) {
            sum += val;
        }
        return sum / data.size();
    }
};