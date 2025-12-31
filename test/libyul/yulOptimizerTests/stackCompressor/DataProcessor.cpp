
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

class DataProcessor {
public:
    static std::vector<int> parseStringToIntVector(const std::string& input) {
        std::vector<int> result;
        std::string temp;
        
        for (char ch : input) {
            if (std::isdigit(ch)) {
                temp += ch;
            } else if (!temp.empty()) {
                result.push_back(std::stoi(temp));
                temp.clear();
            }
        }
        
        if (!temp.empty()) {
            result.push_back(std::stoi(temp));
        }
        
        return result;
    }
    
    static bool validateData(const std::vector<int>& data) {
        if (data.empty()) {
            return false;
        }
        
        for (int value : data) {
            if (value < 0 || value > 1000) {
                return false;
            }
        }
        
        return true;
    }
    
    static std::vector<int> transformData(const std::vector<int>& data) {
        std::vector<int> transformed = data;
        
        for (int& value : transformed) {
            if (value % 2 == 0) {
                value *= 2;
            } else {
                value += 5;
            }
        }
        
        std::sort(transformed.begin(), transformed.end());
        return transformed;
    }
    
    static void printVector(const std::vector<int>& vec) {
        std::cout << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << vec[i];
            if (i < vec.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
};

int main() {
    std::string sampleInput = "data: 10, 25, 7, 42, 99, 123";
    
    std::vector<int> parsedData = DataProcessor::parseStringToIntVector(sampleInput);
    
    std::cout << "Parsed data: ";
    DataProcessor::printVector(parsedData);
    
    if (DataProcessor::validateData(parsedData)) {
        std::cout << "Data validation passed." << std::endl;
        
        std::vector<int> transformedData = DataProcessor::transformData(parsedData);
        
        std::cout << "Transformed data: ";
        DataProcessor::printVector(transformedData);
    } else {
        std::cout << "Data validation failed." << std::endl;
    }
    
    return 0;
}