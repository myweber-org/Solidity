
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

class JsonDataProcessor {
private:
    std::string rawData;
    std::vector<std::string> parsedTokens;
    bool isValidFlag;

    bool containsValidCharacters(const std::string& str) {
        for (char c : str) {
            if (!std::isalnum(c) && c != '_' && c != '-' && c != '.' && c != ':') {
                return false;
            }
        }
        return true;
    }

    void tokenizeData() {
        std::istringstream stream(rawData);
        std::string token;
        while (std::getline(stream, token, ',')) {
            token.erase(0, token.find_first_not_of(" \t\n\r"));
            token.erase(token.find_last_not_of(" \t\n\r") + 1);
            if (!token.empty()) {
                parsedTokens.push_back(token);
            }
        }
    }

public:
    JsonDataProcessor() : isValidFlag(false) {}

    void loadData(const std::string& data) {
        rawData = data;
        parsedTokens.clear();
        isValidFlag = false;
        processData();
    }

    void processData() {
        if (rawData.empty()) {
            isValidFlag = false;
            return;
        }

        tokenizeData();

        if (parsedTokens.empty()) {
            isValidFlag = false;
            return;
        }

        for (const auto& token : parsedTokens) {
            if (!containsValidCharacters(token)) {
                isValidFlag = false;
                return;
            }
        }

        isValidFlag = true;
    }

    bool isValid() const {
        return isValidFlag;
    }

    const std::vector<std::string>& getTokens() const {
        return parsedTokens;
    }

    void displayResults() const {
        if (isValidFlag) {
            std::cout << "Data validation successful." << std::endl;
            std::cout << "Parsed tokens: ";
            for (size_t i = 0; i < parsedTokens.size(); ++i) {
                std::cout << parsedTokens[i];
                if (i != parsedTokens.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
        } else {
            std::cout << "Data validation failed." << std::endl;
        }
    }
};

int main() {
    JsonDataProcessor processor;
    
    std::string testData1 = "name:john, age:25, city:new_york";
    processor.loadData(testData1);
    processor.displayResults();

    std::string testData2 = "invalid@data, corrupt#entry, valid_entry";
    processor.loadData(testData2);
    processor.displayResults();

    return 0;
}