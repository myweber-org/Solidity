
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

class DataProcessor {
public:
    static std::vector<double> normalizeData(const std::vector<double>& input) {
        if (input.empty()) {
            return {};
        }

        double sum = 0.0;
        double sq_sum = 0.0;

        for (double val : input) {
            sum += val;
            sq_sum += val * val;
        }

        double mean = sum / input.size();
        double variance = (sq_sum / input.size()) - (mean * mean);
        double stddev = std::sqrt(variance);

        if (stddev < 1e-10) {
            return std::vector<double>(input.size(), 0.0);
        }

        std::vector<double> normalized;
        normalized.reserve(input.size());

        for (double val : input) {
            normalized.push_back((val - mean) / stddev);
        }

        return normalized;
    }

    static bool validateData(const std::vector<double>& data, double minVal, double maxVal) {
        return std::all_of(data.begin(), data.end(),
            [minVal, maxVal](double val) {
                return !std::isnan(val) && !std::isinf(val) && val >= minVal && val <= maxVal;
            });
    }

    static void removeOutliers(std::vector<double>& data, double threshold) {
        if (data.size() < 2) return;

        auto normalized = normalizeData(data);
        auto it = data.begin();
        auto norm_it = normalized.begin();

        while (it != data.end()) {
            if (std::abs(*norm_it) > threshold) {
                it = data.erase(it);
                norm_it = normalized.erase(norm_it);
            } else {
                ++it;
                ++norm_it;
            }
        }
    }
};

int main() {
    std::vector<double> testData = {1.2, 2.3, 3.4, 4.5, 5.6, 100.0};

    std::cout << "Original data: ";
    for (double val : testData) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    if (DataProcessor::validateData(testData, -1000.0, 1000.0)) {
        std::cout << "Data validation passed." << std::endl;
    } else {
        std::cout << "Data validation failed." << std::endl;
    }

    DataProcessor::removeOutliers(testData, 2.0);

    std::cout << "Data after outlier removal: ";
    for (double val : testData) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    auto normalized = DataProcessor::normalizeData(testData);

    std::cout << "Normalized data: ";
    for (double val : normalized) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
}