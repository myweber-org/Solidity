#include <vector>

std::vector<std::vector<int>> transposeMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) return {};
    
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    
    std::vector<std::vector<int>> transposed(cols, std::vector<int>(rows));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            transposed[j][i] = matrix[i][j];
        }
    }
    
    return transposed;
}