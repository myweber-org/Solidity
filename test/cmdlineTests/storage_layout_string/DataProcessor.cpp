
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

class DataProcessor {
private:
    std::vector<double> data;

public:
    DataProcessor() = default;
    
    explicit DataProcessor(const std::vector<double>& input) : data(input) {}
    
    void addValue(double value) {
        data.push_back(value);
    }
    
    void clearData() {
        data.clear();
    }
    
    double calculateMean() const {
        if (data.empty()) return 0.0;
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        return sum / data.size();
    }
    
    double calculateMedian() {
        if (data.empty()) return 0.0;
        std::vector<double> sortedData = data;
        std::sort(sortedData.begin(), sortedData.end());
        size_t size = sortedData.size();
        
        if (size % 2 == 0) {
            return (sortedData[size/2 - 1] + sortedData[size/2]) / 2.0;
        } else {
            return sortedData[size/2];
        }
    }
    
    double calculateStandardDeviation() const {
        if (data.size() <= 1) return 0.0;
        double mean = calculateMean();
        double sumSquaredDiff = 0.0;
        
        for (double value : data) {
            double diff = value - mean;
            sumSquaredDiff += diff * diff;
        }
        
        return std::sqrt(sumSquaredDiff / (data.size() - 1));
    }
    
    double findMinimum() const {
        if (data.empty()) return 0.0;
        return *std::min_element(data.begin(), data.end());
    }
    
    double findMaximum() const {
        if (data.empty()) return 0.0;
        return *std::max_element(data.begin(), data.end());
    }
    
    size_t getDataSize() const {
        return data.size();
    }
    
    const std::vector<double>& getData() const {
        return data;
    }
};