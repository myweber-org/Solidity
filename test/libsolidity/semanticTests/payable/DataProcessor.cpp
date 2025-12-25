#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

class DataProcessor {
private:
    std::vector<double> dataset;

public:
    DataProcessor(const std::vector<double>& data) : dataset(data) {}

    void addData(double value) {
        dataset.push_back(value);
    }

    double calculateMean() const {
        if (dataset.empty()) return 0.0;
        double sum = std::accumulate(dataset.begin(), dataset.end(), 0.0);
        return sum / dataset.size();
    }

    double calculateMedian() {
        if (dataset.empty()) return 0.0;
        std::vector<double> sortedData = dataset;
        std::sort(sortedData.begin(), sortedData.end());
        size_t size = sortedData.size();
        if (size % 2 == 0) {
            return (sortedData[size/2 - 1] + sortedData[size/2]) / 2.0;
        } else {
            return sortedData[size/2];
        }
    }

    double calculateStandardDeviation() const {
        if (dataset.size() < 2) return 0.0;
        double mean = calculateMean();
        double sumSquaredDiff = 0.0;
        for (double value : dataset) {
            double diff = value - mean;
            sumSquaredDiff += diff * diff;
        }
        return std::sqrt(sumSquaredDiff / (dataset.size() - 1));
    }

    double findMinimum() const {
        if (dataset.empty()) return 0.0;
        return *std::min_element(dataset.begin(), dataset.end());
    }

    double findMaximum() const {
        if (dataset.empty()) return 0.0;
        return *std::max_element(dataset.begin(), dataset.end());
    }

    size_t getDataCount() const {
        return dataset.size();
    }

    void clearData() {
        dataset.clear();
    }

    const std::vector<double>& getDataset() const {
        return dataset;
    }
};