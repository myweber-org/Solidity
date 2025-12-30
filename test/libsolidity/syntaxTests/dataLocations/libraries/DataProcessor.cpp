
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

class DataProcessor {
public:
    using FilterFunc = std::function<bool(int)>;
    using TransformFunc = std::function<int(int)>;

    void addData(int value) {
        rawData.push_back(value);
    }

    void setFilter(FilterFunc func) {
        filter = func;
    }

    void setTransformer(TransformFunc func) {
        transformer = func;
    }

    std::vector<int> process() {
        std::vector<int> result;
        
        for (int value : rawData) {
            if (filter && !filter(value)) {
                continue;
            }
            
            int processedValue = transformer ? transformer(value) : value;
            result.push_back(processedValue);
        }
        
        return result;
    }

    void clear() {
        rawData.clear();
    }

private:
    std::vector<int> rawData;
    FilterFunc filter;
    TransformFunc transformer;
};

bool isEven(int n) {
    return n % 2 == 0;
}

int square(int n) {
    return n * n;
}

int main() {
    DataProcessor processor;
    
    for (int i = 1; i <= 10; ++i) {
        processor.addData(i);
    }
    
    processor.setFilter(isEven);
    processor.setTransformer(square);
    
    std::vector<int> result = processor.process();
    
    std::cout << "Processed data: ";
    for (int value : result) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    return 0;
}