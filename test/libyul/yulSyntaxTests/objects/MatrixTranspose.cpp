#include <iostream>
#include <vector>

std::vector<std::vector<int>> transposeMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) return {};
    size_t n = matrix.size();
    std::vector<std::vector<int>> result(n, std::vector<int>(n, 0));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    return result;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> mat = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    std::cout << "Original matrix:" << std::endl;
    printMatrix(mat);
    std::vector<std::vector<int>> transposed = transposeMatrix(mat);
    std::cout << "Transposed matrix:" << std::endl;
    printMatrix(transposed);
    return 0;
}