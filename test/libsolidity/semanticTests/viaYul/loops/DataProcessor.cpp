
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>

class DataProcessor {
private:
    std::vector<int> data;

public:
    void addData(int value) {
        data.push_back(value);
    }

    std::vector<int> filter(std::function<bool(int)> predicate) const {
        std::vector<int> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result), predicate);
        return result;
    }

    std::vector<int> transform(std::function<int(int)> transformer) const {
        std::vector<int> result;
        result.reserve(data.size());
        std::transform(data.begin(), data.end(), std::back_inserter(result), transformer);
        return result;
    }

    void processPipeline() {
        auto filtered = filter([](int x) { return x % 2 == 0; });
        auto transformed = transform([](int x) { return x * 2; });

        std::cout << "Filtered and transformed data: ";
        for (int val : transformed) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    void display() const {
        std::cout << "Current data: ";
        for (int val : data) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
};

int main() {
    DataProcessor processor;
    processor.addData(1);
    processor.addData(2);
    processor.addData(3);
    processor.addData(4);
    processor.addData(5);

    processor.display();
    processor.processPipeline();

    return 0;
}