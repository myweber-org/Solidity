
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

class DataProcessor {
private:
    std::vector<int> data;

public:
    DataProcessor(const std::vector<int>& input) : data(input) {}

    void filter(std::function<bool(int)> predicate) {
        std::vector<int> filtered;
        std::copy_if(data.begin(), data.end(), std::back_inserter(filtered), predicate);
        data = filtered;
    }

    void transform(std::function<int(int)> transformer) {
        std::transform(data.begin(), data.end(), data.begin(), transformer);
    }

    void sort(bool ascending = true) {
        if (ascending) {
            std::sort(data.begin(), data.end());
        } else {
            std::sort(data.begin(), data.end(), std::greater<int>());
        }
    }

    void print() const {
        for (const auto& value : data) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    std::vector<int> getData() const {
        return data;
    }
};

int main() {
    std::vector<int> numbers = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    DataProcessor processor(numbers);

    std::cout << "Original data: ";
    processor.print();

    processor.filter([](int x) { return x > 3; });
    std::cout << "After filtering (>3): ";
    processor.print();

    processor.transform([](int x) { return x * 2; });
    std::cout << "After transformation (x2): ";
    processor.print();

    processor.sort();
    std::cout << "After ascending sort: ";
    processor.print();

    processor.sort(false);
    std::cout << "After descending sort: ";
    processor.print();

    return 0;
}