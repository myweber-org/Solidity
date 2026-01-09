
#include <vector>
#include <stdexcept>

std::vector<std::vector<int>> transposeMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) {
        return {};
    }

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    for (size_t i = 1; i < rows; ++i) {
        if (matrix[i].size() != cols) {
            throw std::invalid_argument("Input matrix must be rectangular");
        }
    }

    std::vector<std::vector<int>> result(cols, std::vector<int>(rows));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }

    return result;
}