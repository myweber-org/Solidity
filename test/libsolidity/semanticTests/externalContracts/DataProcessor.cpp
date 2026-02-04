
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

class DataProcessor {
public:
    static std::vector<double> normalizeData(const std::vector<double>& rawData, double minRange = 0.0, double maxRange = 1.0) {
        if (rawData.empty()) {
            return {};
        }

        double minVal = *std::min_element(rawData.begin(), rawData.end());
        double maxVal = *std::max_element(rawData.begin(), rawData.end());

        if (std::fabs(maxVal - minVal) < std::numeric_limits<double>::epsilon()) {
            return std::vector<double>(rawData.size(), (minRange + maxRange) / 2.0);
        }

        std::vector<double> normalized;
        normalized.reserve(rawData.size());

        for (double value : rawData) {
            double normalizedValue = minRange + (value - minVal) * (maxRange - minRange) / (maxVal - minVal);
            normalized.push_back(normalizedValue);
        }

        return normalized;
    }

    static bool validateSensorReading(double reading, double minThreshold, double maxThreshold) {
        if (std::isnan(reading) || std::isinf(reading)) {
            return false;
        }
        return (reading >= minThreshold && reading <= maxThreshold);
    }

    static std::vector<double> filterOutliers(const std::vector<double>& data, double zScoreThreshold = 3.0) {
        if (data.size() < 2) {
            return data;
        }

        double sum = 0.0;
        for (double val : data) {
            sum += val;
        }
        double mean = sum / data.size();

        double variance = 0.0;
        for (double val : data) {
            variance += (val - mean) * (val - mean);
        }
        double stdDev = std::sqrt(variance / data.size());

        if (stdDev < std::numeric_limits<double>::epsilon()) {
            return data;
        }

        std::vector<double> filteredData;
        filteredData.reserve(data.size());

        for (double val : data) {
            double zScore = std::fabs((val - mean) / stdDev);
            if (zScore <= zScoreThreshold) {
                filteredData.push_back(val);
            }
        }

        return filteredData;
    }

    static void printStatistics(const std::vector<double>& data) {
        if (data.empty()) {
            std::cout << "No data available for statistics." << std::endl;
            return;
        }

        double sum = 0.0;
        double minVal = std::numeric_limits<double>::max();
        double maxVal = std::numeric_limits<double>::lowest();

        for (double val : data) {
            sum += val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }

        double mean = sum / data.size();

        double variance = 0.0;
        for (double val : data) {
            variance += (val - mean) * (val - mean);
        }
        double stdDev = std::sqrt(variance / data.size());

        std::cout << "Statistics:" << std::endl;
        std::cout << "  Count: " << data.size() << std::endl;
        std::cout << "  Minimum: " << minVal << std::endl;
        std::cout << "  Maximum: " << maxVal << std::endl;
        std::cout << "  Mean: " << mean << std::endl;
        std::cout << "  Standard Deviation: " << stdDev << std::endl;
    }
};

int main() {
    std::vector<double> sensorReadings = {12.5, 13.2, 11.8, 15.3, 10.5, 100.0, 12.9, 14.1};

    std::cout << "Original data:" << std::endl;
    for (double val : sensorReadings) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    DataProcessor::printStatistics(sensorReadings);

    std::vector<double> filtered = DataProcessor::filterOutliers(sensorReadings);
    std::cout << "\nFiltered data (outliers removed):" << std::endl;
    for (double val : filtered) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::vector<double> normalized = DataProcessor::normalizeData(filtered);
    std::cout << "\nNormalized data (0 to 1 range):" << std::endl;
    for (double val : normalized) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "\nValidation checks:" << std::endl;
    for (double val : sensorReadings) {
        bool valid = DataProcessor::validateSensorReading(val, 10.0, 20.0);
        std::cout << "Value " << val << ": " << (valid ? "VALID" : "INVALID") << std::endl;
    }

    return 0;
}