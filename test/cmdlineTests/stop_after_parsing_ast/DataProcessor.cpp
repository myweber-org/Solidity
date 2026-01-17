
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class DataProcessor {
public:
    static std::vector<double> validateAndNormalize(const std::vector<double>& rawData, double minValid, double maxValid) {
        if (rawData.empty()) {
            throw std::invalid_argument("Input data cannot be empty");
        }

        if (minValid >= maxValid) {
            throw std::invalid_argument("Minimum value must be less than maximum value");
        }

        std::vector<double> processedData;
        processedData.reserve(rawData.size());

        for (double value : rawData) {
            if (value < minValid || value > maxValid) {
                throw std::out_of_range("Data value out of valid range");
            }
            processedData.push_back(value);
        }

        double mean = calculateMean(processedData);
        double stdDev = calculateStandardDeviation(processedData, mean);

        if (stdDev == 0.0) {
            return processedData;
        }

        for (double& value : processedData) {
            value = (value - mean) / stdDev;
        }

        return processedData;
    }

private:
    static double calculateMean(const std::vector<double>& data) {
        double sum = 0.0;
        for (double value : data) {
            sum += value;
        }
        return sum / data.size();
    }

    static double calculateStandardDeviation(const std::vector<double>& data, double mean) {
        double variance = 0.0;
        for (double value : data) {
            variance += (value - mean) * (value - mean);
        }
        return std::sqrt(variance / data.size());
    }
};