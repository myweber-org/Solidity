#include <iostream>
#include <vector>

template<typename T>
std::vector<std::vector<T>> transposeMatrix(const std::vector<std::vector<T>>& matrix) {
    if (matrix.empty()) return {};
    
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    
    std::vector<std::vector<T>> result(cols, std::vector<T>(rows));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    
    return result;
}

void printMatrix(const auto& matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> original = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    std::cout << "Original matrix:" << std::endl;
    printMatrix(original);
    
    auto transposed = transposeMatrix(original);
    
    std::cout << "\nTransposed matrix:" << std::endl;
    printMatrix(transposed);
    
    return 0;
}