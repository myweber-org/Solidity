
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

class DataProcessor {
private:
    std::vector<int> data;

public:
    DataProcessor(const std::vector<int>& input) : data(input) {}

    DataProcessor& filter(const std::function<bool(int)>& predicate) {
        std::vector<int> filtered;
        std::copy_if(data.begin(), data.end(), std::back_inserter(filtered), predicate);
        data = filtered;
        return *this;
    }

    DataProcessor& transform(const std::function<int(int)>& transformer) {
        std::transform(data.begin(), data.end(), data.begin(), transformer);
        return *this;
    }

    DataProcessor& sort(bool ascending = true) {
        if (ascending) {
            std::sort(data.begin(), data.end());
        } else {
            std::sort(data.begin(), data.end(), std::greater<int>());
        }
        return *this;
    }

    void print() const {
        for (const auto& value : data) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    std::vector<int> getResult() const {
        return data;
    }
};

int main() {
    std::vector<int> numbers = {5, 2, 8, 3, 1, 9, 4, 7, 6};

    DataProcessor processor(numbers);
    processor.filter([](int x) { return x % 2 == 0; })
             .transform([](int x) { return x * 2; })
             .sort()
             .print();

    return 0;
}