
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

class DataProcessor {
public:
    using FilterFunc = std::function<bool(int)>;
    using TransformFunc = std::function<int(int)>;

    void addFilter(FilterFunc filter) {
        filters.push_back(filter);
    }

    void addTransform(TransformFunc transform) {
        transforms.push_back(transform);
    }

    std::vector<int> process(const std::vector<int>& input) const {
        std::vector<int> result;
        
        for (int value : input) {
            bool passed = true;
            
            for (const auto& filter : filters) {
                if (!filter(value)) {
                    passed = false;
                    break;
                }
            }
            
            if (passed) {
                int transformedValue = value;
                for (const auto& transform : transforms) {
                    transformedValue = transform(transformedValue);
                }
                result.push_back(transformedValue);
            }
        }
        
        return result;
    }

private:
    std::vector<FilterFunc> filters;
    std::vector<TransformFunc> transforms;
};

int main() {
    DataProcessor processor;
    
    processor.addFilter([](int x) { return x > 0; });
    processor.addFilter([](int x) { return x % 2 == 0; });
    
    processor.addTransform([](int x) { return x * 2; });
    processor.addTransform([](int x) { return x + 10; });
    
    std::vector<int> inputData = {-5, 2, 3, 8, 10, -1, 6};
    std::vector<int> result = processor.process(inputData);
    
    std::cout << "Original data: ";
    for (int val : inputData) {
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