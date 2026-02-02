
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

class DataFilter {
public:
    virtual std::vector<int> process(const std::vector<int>& data) = 0;
    virtual ~DataFilter() = default;
};

class ThresholdFilter : public DataFilter {
private:
    int threshold;
public:
    explicit ThresholdFilter(int t) : threshold(t) {}
    
    std::vector<int> process(const std::vector<int>& data) override {
        std::vector<int> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result),
                    [this](int value) { return value > threshold; });
        return result;
    }
};

class TransformFilter : public DataFilter {
private:
    std::function<int(int)> transformer;
public:
    explicit TransformFilter(std::function<int(int)> t) : transformer(t) {}
    
    std::vector<int> process(const std::vector<int>& data) override {
        std::vector<int> result;
        std::transform(data.begin(), data.end(), std::back_inserter(result), transformer);
        return result;
    }
};

class DataProcessor {
private:
    std::vector<std::unique_ptr<DataFilter>> filters;
    
public:
    void addFilter(std::unique_ptr<DataFilter> filter) {
        filters.push_back(std::move(filter));
    }
    
    std::vector<int> process(const std::vector<int>& input) {
        std::vector<int> current = input;
        
        for (const auto& filter : filters) {
            current = filter->process(current);
        }
        
        return current;
    }
};

int main() {
    DataProcessor processor;
    
    processor.addFilter(std::make_unique<ThresholdFilter>(10));
    processor.addFilter(std::make_unique<TransformFilter>([](int x) { return x * 2; }));
    
    std::vector<int> testData = {5, 15, 8, 20, 3, 25};
    std::vector<int> result = processor.process(testData);
    
    std::cout << "Original data: ";
    for (int val : testData) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Processed data: ";
    for (int val : result) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    return 0;
}