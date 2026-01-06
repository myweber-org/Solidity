#include <vector>
#include <stdexcept>

std::vector<std::vector<double>> transposeMatrix(const std::vector<std::vector<double>>& matrix) {
    if (matrix.empty()) {
        return {};
    }
    size_t n = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != n) {
            throw std::invalid_argument("Input must be a square matrix.");
        }
    }
    std::vector<std::vector<double>> result(n, std::vector<double>(n, 0.0));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    return result;
}