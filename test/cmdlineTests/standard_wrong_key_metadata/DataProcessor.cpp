
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class SensorDataProcessor {
public:
    static std::vector<double> validateAndNormalize(const std::vector<double>& rawReadings, double minValid, double maxValid) {
        if (rawReadings.empty()) {
            throw std::invalid_argument("Input data cannot be empty");
        }
        
        if (minValid >= maxValid) {
            throw std::invalid_argument("Invalid range: min must be less than max");
        }
        
        std::vector<double> processed;
        processed.reserve(rawReadings.size());
        
        for (double reading : rawReadings) {
            if (reading < minValid || reading > maxValid) {
                throw std::out_of_range("Sensor reading out of valid range");
            }
            processed.push_back(reading);
        }
        
        double sum = 0.0;
        for (double val : processed) {
            sum += val;
        }
        double mean = sum / processed.size();
        
        double variance = 0.0;
        for (double val : processed) {
            variance += std::pow(val - mean, 2);
        }
        variance /= processed.size();
        double stdDev = std::sqrt(variance);
        
        if (stdDev == 0.0) {
            return processed;
        }
        
        std::vector<double> normalized;
        normalized.reserve(processed.size());
        for (double val : processed) {
            normalized.push_back((val - mean) / stdDev);
        }
        
        return normalized;
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