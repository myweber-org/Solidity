
#include <vector>
#include <algorithm>
#include <functional>

class DataProcessor {
private:
    std::vector<int> dataset;

public:
    explicit DataProcessor(const std::vector<int>& initialData) : dataset(initialData) {}

    void addData(int value) {
        dataset.push_back(value);
    }

    void filterData(std::function<bool(int)> predicate) {
        std::vector<int> filtered;
        std::copy_if(dataset.begin(), dataset.end(), std::back_inserter(filtered), predicate);
        dataset = filtered;
    }

    void sortData(bool ascending = true) {
        if (ascending) {
            std::sort(dataset.begin(), dataset.end());
        } else {
            std::sort(dataset.begin(), dataset.end(), std::greater<int>());
        }
    }

    void removeDuplicates() {
        std::sort(dataset.begin(), dataset.end());
        auto last = std::unique(dataset.begin(), dataset.end());
        dataset.erase(last, dataset.end());
    }

    std::vector<int> getProcessedData() const {
        return dataset;
    }

    size_t getDataSize() const {
        return dataset.size();
    }

    void clearData() {
        dataset.clear();
    }
};