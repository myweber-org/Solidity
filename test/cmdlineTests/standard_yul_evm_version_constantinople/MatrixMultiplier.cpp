#include <iostream>
#include <vector>
#include <stdexcept>

template<typename T>
std::vector<std::vector<T>> multiplyMatrices(const std::vector<std::vector<T>>& matrixA,
                                             const std::vector<std::vector<T>>& matrixB) {
    if (matrixA.empty() || matrixB.empty()) {
        throw std::invalid_argument("Input matrices cannot be empty.");
    }
    size_t rowsA = matrixA.size();
    size_t colsA = matrixA[0].size();
    size_t rowsB = matrixB.size();
    size_t colsB = matrixB[0].size();

    for (const auto& row : matrixA) {
        if (row.size() != colsA) {
            throw std::invalid_argument("Matrix A rows must have consistent size.");
        }
    }
    for (const auto& row : matrixB) {
        if (row.size() != colsB) {
            throw std::invalid_argument("Matrix B rows must have consistent size.");
        }
    }

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix A columns must equal Matrix B rows for multiplication.");
    }

    std::vector<std::vector<T>> result(rowsA, std::vector<T>(colsB, T()));

    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            T sum = T();
            for (size_t k = 0; k < colsA; ++k) {
                sum += matrixA[i][k] * matrixB[k][j];
            }
            result[i][j] = sum;
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
    try {
        std::vector<std::vector<int>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<int>> B = {{7, 8}, {9, 10}, {11, 12}};

        auto C = multiplyMatrices(A, B);
        std::cout << "Result matrix:" << std::endl;
        printMatrix(C);

        std::vector<std::vector<double>> D = {{1.5, 2.5}, {3.5, 4.5}};
        std::vector<std::vector<double>> E = {{0.5, 1.0}, {1.5, 2.0}};

        auto F = multiplyMatrices(D, E);
        std::cout << "\nDouble result matrix:" << std::endl;
        printMatrix(F);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}