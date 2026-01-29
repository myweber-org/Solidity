#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

class CSVDataProcessor {
private:
    std::vector<std::vector<std::string>> data;
    std::vector<std::string> headers;

    std::vector<std::string> splitLine(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(line);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    bool validateRow(const std::vector<std::string>& row) {
        return row.size() == headers.size();
    }

public:
    bool loadFromFile(const std::string& filename, char delimiter = ',') {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string line;
        if (std::getline(file, line)) {
            headers = splitLine(line, delimiter);
        } else {
            return false;
        }

        while (std::getline(file, line)) {
            auto row = splitLine(line, delimiter);
            if (validateRow(row)) {
                data.push_back(row);
            } else {
                std::cerr << "Warning: Skipping invalid row: " << line << std::endl;
            }
        }

        file.close();
        return true;
    }

    void displaySummary() const {
        std::cout << "CSV Data Summary:" << std::endl;
        std::cout << "Headers: " << headers.size() << " columns" << std::endl;
        std::cout << "Rows: " << data.size() << " entries" << std::endl;

        if (!headers.empty()) {
            std::cout << "Column names: ";
            for (size_t i = 0; i < headers.size(); ++i) {
                std::cout << headers[i];
                if (i < headers.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }

    const std::vector<std::vector<std::string>>& getData() const {
        return data;
    }

    const std::vector<std::string>& getHeaders() const {
        return headers;
    }

    std::vector<std::string> getColumn(size_t index) const {
        if (index >= headers.size()) {
            throw std::out_of_range("Column index out of range");
        }

        std::vector<std::string> column;
        for (const auto& row : data) {
            column.push_back(row[index]);
        }
        return column;
    }
};