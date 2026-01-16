
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

        auto minMax = std::minmax_element(rawData.begin(), rawData.end());
        double dataMin = *minMax.first;
        double dataMax = *minMax.second;

        if (std::fabs(dataMax - dataMin) < std::numeric_limits<double>::epsilon()) {
            std::vector<double> normalized(rawData.size(), (minRange + maxRange) / 2.0);
            return normalized;
        }

        std::vector<double> normalized;
        normalized.reserve(rawData.size());
        for (double value : rawData) {
            double normValue = minRange + (value - dataMin) * (maxRange - minRange) / (dataMax - dataMin);
            normalized.push_back(normValue);
        }
        return normalized;
    }

    static bool validateSensorData(const std::vector<double>& data, double lowerBound, double upperBound) {
        if (data.empty()) {
            return false;
        }

        for (double value : data) {
            if (std::isnan(value) || std::isinf(value)) {
                return false;
            }
            if (value < lowerBound || value > upperBound) {
                return false;
            }
        }
        return true;
    }

    static double calculateMovingAverage(const std::vector<double>& data, size_t windowSize) {
        if (data.empty() || windowSize == 0 || windowSize > data.size()) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        double sum = 0.0;
        for (size_t i = 0; i < windowSize; ++i) {
            sum += data[i];
        }
        return sum / windowSize;
    }
};

int main() {
    std::vector<double> sensorReadings = {23.5, 24.1, 22.8, 25.3, 23.9, 24.5};

    if (DataProcessor::validateSensorData(sensorReadings, 20.0, 30.0)) {
        std::cout << "Sensor data validation passed." << std::endl;

        std::vector<double> normalized = DataProcessor::normalizeData(sensorReadings);
        std::cout << "Normalized values: ";
        for (double val : normalized) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        double avg = DataProcessor::calculateMovingAverage(sensorReadings, 3);
        std::cout << "Moving average (window 3): " << avg << std::endl;
    } else {
        std::cout << "Sensor data validation failed." << std::endl;
    }

    return 0;
}