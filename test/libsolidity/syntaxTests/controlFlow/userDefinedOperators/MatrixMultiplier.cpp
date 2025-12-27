
#include <iostream>
#include <vector>
#include <stdexcept>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& matrixA,
                                                     const std::vector<std::vector<double>>& matrixB) {
        if (matrixA.empty() || matrixB.empty()) {
            throw std::invalid_argument("Input matrices cannot be empty");
        }

        size_t rowsA = matrixA.size();
        size_t colsA = matrixA[0].size();
        size_t rowsB = matrixB.size();
        size_t colsB = matrixB[0].size();

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions are incompatible for multiplication");
        }

        for (size_t i = 0; i < rowsA; ++i) {
            if (matrixA[i].size() != colsA) {
                throw std::invalid_argument("Matrix A has inconsistent row sizes");
            }
        }

        for (size_t i = 0; i < rowsB; ++i) {
            if (matrixB[i].size() != colsB) {
                throw std::invalid_argument("Matrix B has inconsistent row sizes");
            }
        }

        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                for (size_t k = 0; k < colsA; ++k) {
                    result[i][j] += matrixA[i][k] * matrixB[k][j];
                }
            }
        }

        return result;
    }

    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        std::vector<std::vector<double>> matrixA = {
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0}
        };

        std::vector<std::vector<double>> matrixB = {
            {7.0, 8.0},
            {9.0, 10.0},
            {11.0, 12.0}
        };

        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(matrixA);

        std::cout << "\nMatrix B:" << std::endl;
        MatrixMultiplier::printMatrix(matrixB);

        std::vector<std::vector<double>> result = MatrixMultiplier::multiply(matrixA, matrixB);

        std::cout << "\nResult of multiplication:" << std::endl;
        MatrixMultiplier::printMatrix(result);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}