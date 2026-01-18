#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>

class DataProcessor {
public:
    DataProcessor() = default;

    static std::vector<double> filterByThreshold(const std::vector<double>& data, double threshold) {
        std::vector<double> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result),
                     [threshold](double value) { return value >= threshold; });
        return result;
    }

    static double calculateAverage(const std::vector<double>& data) {
        if (data.empty()) {
            throw std::invalid_argument("Cannot calculate average of empty dataset");
        }
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        return sum / static_cast<double>(data.size());
    }

    static std::vector<double> normalizeData(const std::vector<double>& data) {
        if (data.empty()) {
            return {};
        }
        
        double min_val = *std::min_element(data.begin(), data.end());
        double max_val = *std::max_element(data.begin(), data.end());
        
        if (max_val == min_val) {
            return std::vector<double>(data.size(), 0.5);
        }
        
        std::vector<double> normalized;
        normalized.reserve(data.size());
        
        for (double value : data) {
            normalized.push_back((value - min_val) / (max_val - min_val));
        }
        
        return normalized;
    }

    static std::vector<double> movingAverage(const std::vector<double>& data, size_t window_size) {
        if (window_size == 0 || window_size > data.size()) {
            throw std::invalid_argument("Invalid window size for moving average");
        }
        
        std::vector<double> result;
        result.reserve(data.size() - window_size + 1);
        
        for (size_t i = 0; i <= data.size() - window_size; ++i) {
            double sum = 0.0;
            for (size_t j = 0; j < window_size; ++j) {
                sum += data[i + j];
            }
            result.push_back(sum / static_cast<double>(window_size));
        }
        
        return result;
    }
};