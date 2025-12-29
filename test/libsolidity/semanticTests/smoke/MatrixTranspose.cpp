#include <iostream>
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

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
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

        std::vector<std::vector<int>> invalid = {
            {1, 2, 3},
            {4, 5}
        };

        std::cout << "\nTesting invalid matrix..." << std::endl;
        std::vector<std::vector<int>> shouldFail = transposeMatrix(invalid);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}