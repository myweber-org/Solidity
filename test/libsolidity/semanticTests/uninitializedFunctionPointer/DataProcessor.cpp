
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

class CSVProcessor {
private:
    std::vector<std::vector<std::string>> data;
    
    std::vector<std::string> splitLine(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    bool validateRow(const std::vector<std::string>& row, size_t expectedColumns) {
        return row.size() == expectedColumns;
    }

public:
    bool loadCSV(const std::string& filename, char delimiter = ',') {
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return false;
        }
        
        std::string line;
        size_t expectedColumns = 0;
        size_t lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            
            if (line.empty()) {
                continue;
            }
            
            std::vector<std::string> row = splitLine(line, delimiter);
            
            if (lineNumber == 1) {
                expectedColumns = row.size();
            }
            
            if (!validateRow(row, expectedColumns)) {
                std::cerr << "Error: Invalid column count at line " << lineNumber << std::endl;
                return false;
            }
            
            data.push_back(row);
        }
        
        file.close();
        return true;
    }
    
    void displayData() const {
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << row[i];
                if (i < row.size() - 1) {
                    std::cout << " | ";
                }
            }
            std::cout << std::endl;
        }
    }
    
    size_t getRowCount() const {
        return data.size();
    }
    
    size_t getColumnCount() const {
        return data.empty() ? 0 : data[0].size();
    }
    
    std::vector<std::string> getColumn(size_t columnIndex) const {
        std::vector<std::string> column;
        
        if (columnIndex >= getColumnCount()) {
            return column;
        }
        
        for (const auto& row : data) {
            column.push_back(row[columnIndex]);
        }
        
        return column;
    }
    
    void sortByColumn(size_t columnIndex) {
        if (columnIndex >= getColumnCount() || data.empty()) {
            return;
        }
        
        std::sort(data.begin(), data.end(),
            [columnIndex](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                return a[columnIndex] < b[columnIndex];
            });
    }
};

int main() {
    CSVProcessor processor;
    
    if (processor.loadCSV("data.csv")) {
        std::cout << "Data loaded successfully!" << std::endl;
        std::cout << "Rows: " << processor.getRowCount() << std::endl;
        std::cout << "Columns: " << processor.getColumnCount() << std::endl;
        
        std::cout << "\nOriginal Data:" << std::endl;
        processor.displayData();
        
        processor.sortByColumn(0);
        
        std::cout << "\nSorted Data:" << std::endl;
        processor.displayData();
    }
    
    return 0;
}