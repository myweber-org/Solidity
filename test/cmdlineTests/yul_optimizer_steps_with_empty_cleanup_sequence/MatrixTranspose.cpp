
#include <vector>

std::vector<std::vector<double>> transposeMatrix(const std::vector<std::vector<double>>& matrix) {
    if (matrix.empty()) return {};
    
    size_t rows = matrix.size();
    size_t cols = matrix[0].size();
    
    std::vector<std::vector<double>> result(cols, std::vector<double>(rows));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    
    return result;
}