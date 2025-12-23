#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class DataProcessor {
public:
    static std::vector<double> validateAndNormalize(const std::vector<double>& rawData, double minValid, double maxValid) {
        if (rawData.empty()) {
            throw std::invalid_argument("Input data vector is empty");
        }
        
        if (minValid >= maxValid) {
            throw std::invalid_argument("Invalid range: min must be less than max");
        }
        
        std::vector<double> processedData;
        processedData.reserve(rawData.size());
        
        for (double value : rawData) {
            if (value < minValid || value > maxValid) {
                throw std::out_of_range("Data value out of valid range");
            }
            processedData.push_back(value);
        }
        
        double sum = 0.0;
        for (double value : processedData) {
            sum += value;
        }
        double mean = sum / processedData.size();
        
        double variance = 0.0;
        for (double value : processedData) {
            variance += std::pow(value - mean, 2);
        }
        variance /= processedData.size();
        double stdDev = std::sqrt(variance);
        
        if (stdDev == 0.0) {
            return processedData;
        }
        
        for (size_t i = 0; i < processedData.size(); ++i) {
            processedData[i] = (processedData[i] - mean) / stdDev;
        }
        
        return processedData;
    }
    
    static double calculateMovingAverage(const std::vector<double>& data, size_t windowSize) {
        if (data.empty()) {
            throw std::invalid_argument("Data vector is empty");
        }
        
        if (windowSize == 0 || windowSize > data.size()) {
            windowSize = data.size();
        }
        
        double sum = 0.0;
        size_t startIdx = data.size() - windowSize;
        
        for (size_t i = startIdx; i < data.size(); ++i) {
            sum += data[i];
        }
        
        return sum / windowSize;
    }
};