
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

class DataFilter {
public:
    virtual ~DataFilter() = default;
    virtual std::vector<int> process(const std::vector<int>& data) = 0;
};

class ThresholdFilter : public DataFilter {
private:
    int threshold_;
public:
    explicit ThresholdFilter(int threshold) : threshold_(threshold) {}
    
    std::vector<int> process(const std::vector<int>& data) override {
        std::vector<int> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result),
                    [this](int value) { return value > threshold_; });
        return result;
    }
};

class DataTransformer {
public:
    virtual ~DataTransformer() = default;
    virtual std::vector<int> transform(const std::vector<int>& data) = 0;
};

class ScaleTransformer : public DataTransformer {
private:
    int factor_;
public:
    explicit ScaleTransformer(int factor) : factor_(factor) {}
    
    std::vector<int> transform(const std::vector<int>& data) override {
        std::vector<int> result;
        result.reserve(data.size());
        std::transform(data.begin(), data.end(), std::back_inserter(result),
                      [this](int value) { return value * factor_; });
        return result;
    }
};

class DataProcessingPipeline {
private:
    std::vector<std::unique_ptr<DataFilter>> filters_;
    std::vector<std::unique_ptr<DataTransformer>> transformers_;
    
public:
    void addFilter(std::unique_ptr<DataFilter> filter) {
        filters_.push_back(std::move(filter));
    }
    
    void addTransformer(std::unique_ptr<DataTransformer> transformer) {
        transformers_.push_back(std::move(transformer));
    }
    
    std::vector<int> process(const std::vector<int>& input) {
        std::vector<int> result = input;
        
        for (const auto& filter : filters_) {
            result = filter->process(result);
        }
        
        for (const auto& transformer : transformers_) {
            result = transformer->transform(result);
        }
        
        return result;
    }
    
    void clear() {
        filters_.clear();
        transformers_.clear();
    }
};

void printVector(const std::vector<int>& vec, const std::string& label) {
    std::cout << label << ": ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
}

int main() {
    DataProcessingPipeline pipeline;
    
    pipeline.addFilter(std::make_unique<ThresholdFilter>(10));
    pipeline.addTransformer(std::make_unique<ScaleTransformer>(2));
    
    std::vector<int> testData = {5, 15, 8, 20, 3, 25, 12, 18};
    
    printVector(testData, "Original data");
    
    std::vector<int> processedData = pipeline.process(testData);
    
    printVector(processedData, "Processed data");
    
    pipeline.clear();
    
    pipeline.addFilter(std::make_unique<ThresholdFilter>(15));
    pipeline.addTransformer(std::make_unique<ScaleTransformer>(3));
    
    std::vector<int> secondPass = pipeline.process(testData);
    
    printVector(secondPass, "Second pass");
    
    return 0;
}