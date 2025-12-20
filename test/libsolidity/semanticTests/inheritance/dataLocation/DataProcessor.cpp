
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

class DataProcessor {
public:
    static std::string trimWhitespace(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(first, (last - first + 1));
    }

    static std::string toLowerCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    static bool isValidEmail(const std::string& email) {
        size_t at_pos = email.find('@');
        if (at_pos == std::string::npos || at_pos == 0) return false;
        
        size_t dot_pos = email.find('.', at_pos);
        if (dot_pos == std::string::npos || dot_pos <= at_pos + 1) return false;
        
        return dot_pos < email.length() - 1;
    }

    static std::vector<int> filterEvenNumbers(const std::vector<int>& numbers) {
        std::vector<int> result;
        std::copy_if(numbers.begin(), numbers.end(), std::back_inserter(result),
                     [](int n) { return n % 2 == 0; });
        return result;
    }

    static void removeDuplicates(std::vector<std::string>& items) {
        std::sort(items.begin(), items.end());
        auto last = std::unique(items.begin(), items.end());
        items.erase(last, items.end());
    }
};

int main() {
    std::string test_string = "  Hello World  ";
    std::cout << "Trimmed: '" << DataProcessor::trimWhitespace(test_string) << "'" << std::endl;
    
    std::string uppercase = "HELLO";
    std::cout << "Lowercase: " << DataProcessor::toLowerCase(uppercase) << std::endl;
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> evens = DataProcessor::filterEvenNumbers(numbers);
    std::cout << "Even numbers: ";
    for (int n : evens) std::cout << n << " ";
    std::cout << std::endl;
    
    std::vector<std::string> items = {"apple", "orange", "apple", "banana", "orange"};
    DataProcessor::removeDuplicates(items);
    std::cout << "Unique items: ";
    for (const auto& item : items) std::cout << item << " ";
    std::cout << std::endl;
    
    return 0;
}