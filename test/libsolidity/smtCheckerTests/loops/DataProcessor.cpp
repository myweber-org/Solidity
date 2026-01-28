
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class DataProcessor {
public:
    static std::vector<double> normalize(const std::vector<double>& input) {
        if (input.empty()) {
            throw std::invalid_argument("Input vector cannot be empty");
        }

        double min_val = *std::min_element(input.begin(), input.end());
        double max_val = *std::max_element(input.begin(), input.end());

        if (std::abs(max_val - min_val) < 1e-10) {
            return std::vector<double>(input.size(), 0.5);
        }

        std::vector<double> normalized;
        normalized.reserve(input.size());

        for (double value : input) {
            normalized.push_back((value - min_val) / (max_val - min_val));
        }

        return normalized;
    }

    static bool validateSensorData(const std::vector<double>& data, double min_range, double max_range) {
        for (double value : data) {
            if (value < min_range || value > max_range || std::isnan(value)) {
                return false;
            }
        }
        return true;
    }

    static double calculateMovingAverage(const std::vector<double>& data, size_t window_size) {
        if (data.empty() || window_size == 0 || window_size > data.size()) {
            throw std::invalid_argument("Invalid parameters for moving average");
        }

        double sum = 0.0;
        for (size_t i = 0; i < window_size; ++i) {
            sum += data[i];
        }

        return sum / window_size;
    }
};