
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

class DataProcessor {
public:
    static std::vector<double> normalizeData(const std::vector<double>& rawData) {
        if (rawData.empty()) {
            return {};
        }

        auto minMax = std::minmax_element(rawData.begin(), rawData.end());
        double minVal = *minMax.first;
        double maxVal = *minMax.second;
        double range = maxVal - minVal;

        if (std::fabs(range) < std::numeric_limits<double>::epsilon()) {
            return std::vector<double>(rawData.size(), 0.5);
        }

        std::vector<double> normalized;
        normalized.reserve(rawData.size());
        
        for (double value : rawData) {
            normalized.push_back((value - minVal) / range);
        }

        return normalized;
    }

    static bool validateData(const std::vector<double>& data) {
        if (data.empty()) {
            std::cerr << "Error: Empty data set provided." << std::endl;
            return false;
        }

        for (double value : data) {
            if (std::isnan(value) || std::isinf(value)) {
                std::cerr << "Error: Invalid numeric value detected." << std::endl;
                return false;
            }
        }

        return true;
    }

    static void processDataset(std::vector<double>& dataset) {
        if (!validateData(dataset)) {
            std::cerr << "Data validation failed. Processing aborted." << std::endl;
            return;
        }

        std::cout << "Original dataset size: " << dataset.size() << std::endl;
        
        std::vector<double> normalized = normalizeData(dataset);
        
        std::cout << "Normalized values:" << std::endl;
        for (size_t i = 0; i < normalized.size(); ++i) {
            std::cout << "Index " << i << ": " << dataset[i] << " -> " << normalized[i] << std::endl;
        }

        double sum = 0.0;
        for (double val : normalized) {
            sum += val;
        }
        std::cout << "Mean of normalized data: " << sum / normalized.size() << std::endl;
    }
};

int main() {
    std::vector<double> testData = {12.5, 3.7, 25.1, 8.9, 15.3, 6.2, 30.0, 1.5};
    
    std::cout << "Starting data processing pipeline..." << std::endl;
    DataProcessor::processDataset(testData);
    
    std::vector<double> edgeCase = {5.0, 5.0, 5.0};
    std::cout << "\nProcessing edge case (constant values):" << std::endl;
    DataProcessor::processDataset(edgeCase);
    
    return 0;
}