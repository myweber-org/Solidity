#include <iostream>
#include <vector>
#include <type_traits>

template<typename T>
std::vector<std::vector<T>> transposeMatrix(const std::vector<std::vector<T>>& matrix) {
    if (matrix.empty()) return {};

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    for (const auto& row : matrix) {
        if (row.size() != cols) {
            throw std::invalid_argument("Input matrix must be rectangular");
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

template<typename T>
void printMatrix(const std::vector<std::vector<T>>& matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> intMatrix = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    std::cout << "Original integer matrix:" << std::endl;
    printMatrix(intMatrix);

    auto transposedInt = transposeMatrix(intMatrix);
    std::cout << "\nTransposed integer matrix:" << std::endl;
    printMatrix(transposedInt);

    std::vector<std::vector<double>> doubleMatrix = {
        {1.1, 2.2},
        {3.3, 4.4},
        {5.5, 6.6}
    };

    std::cout << "\nOriginal double matrix:" << std::endl;
    printMatrix(doubleMatrix);

    auto transposedDouble = transposeMatrix(doubleMatrix);
    std::cout << "\nTransposed double matrix:" << std::endl;
    printMatrix(transposedDouble);

    return 0;
}