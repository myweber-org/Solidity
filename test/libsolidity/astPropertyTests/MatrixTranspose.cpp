
#include <iostream>
#include <vector>

template <typename T>
std::vector<std::vector<T>> transposeMatrix(const std::vector<std::vector<T>>& matrix) {
    if (matrix.empty()) return {};
    
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    
    for (const auto& row : matrix) {
        if (row.size() != cols) {
            throw std::invalid_argument("Matrix must have consistent row sizes");
        }
    }
    
    std::vector<std::vector<T>> result(cols, std::vector<T>(rows));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    
    return result;
}

template <typename T>
void printMatrix(const std::vector<std::vector<T>>& matrix) {
    for (const auto& row : matrix) {
        for (const auto& element : row) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    try {
        std::vector<std::vector<int>> original = {
            {1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };
        
        std::cout << "Original matrix:" << std::endl;
        printMatrix(original);
        
        std::vector<std::vector<int>> transposed = transposeMatrix(original);
        
        std::cout << "\nTransposed matrix:" << std::endl;
        printMatrix(transposed);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}