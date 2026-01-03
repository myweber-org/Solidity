
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace DataUtils {

    std::vector<double> normalizeData(const std::vector<double>& input) {
        if (input.empty()) {
            return {};
        }

        double minVal = *std::min_element(input.begin(), input.end());
        double maxVal = *std::max_element(input.begin(), input.end());

        if (maxVal - minVal < 1e-12) {
            return std::vector<double>(input.size(), 0.5);
        }

        std::vector<double> normalized;
        normalized.reserve(input.size());
        for (double val : input) {
            normalized.push_back((val - minVal) / (maxVal - minVal));
        }
        return normalized;
    }

    bool validateDataRange(const std::vector<double>& data, double lowerBound, double upperBound) {
        if (lowerBound >= upperBound) {
            throw std::invalid_argument("Invalid bounds provided");
        }

        for (double value : data) {
            if (value < lowerBound || value > upperBound) {
                return false;
            }
        }
        return true;
    }

    double computeAverage(const std::vector<double>& data) {
        if (data.empty()) {
            throw std::invalid_argument("Cannot compute average of empty dataset");
        }

        double sum = 0.0;
        for (double val : data) {
            sum += val;
        }
        return sum / static_cast<double>(data.size());
    }
}