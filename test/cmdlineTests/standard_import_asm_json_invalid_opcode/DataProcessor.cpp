
#include <vector>
#include <algorithm>
#include <stdexcept>

class DataProcessor {
private:
    std::vector<int> dataset;

public:
    void addData(int value) {
        dataset.push_back(value);
    }

    void removeData(int index) {
        if (index < 0 || index >= dataset.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        dataset.erase(dataset.begin() + index);
    }

    void sortData(bool ascending = true) {
        if (ascending) {
            std::sort(dataset.begin(), dataset.end());
        } else {
            std::sort(dataset.begin(), dataset.end(), std::greater<int>());
        }
    }

    std::vector<int> filterData(int threshold) const {
        std::vector<int> result;
        std::copy_if(dataset.begin(), dataset.end(), std::back_inserter(result),
                    [threshold](int value) { return value > threshold; });
        return result;
    }

    double calculateAverage() const {
        if (dataset.empty()) {
            throw std::runtime_error("Dataset is empty");
        }
        
        double sum = 0.0;
        for (int value : dataset) {
            sum += value;
        }
        return sum / dataset.size();
    }

    void clearData() {
        dataset.clear();
    }

    size_t getSize() const {
        return dataset.size();
    }

    const std::vector<int>& getDataset() const {
        return dataset;
    }
};