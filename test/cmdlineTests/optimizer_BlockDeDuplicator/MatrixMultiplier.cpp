#include <iostream>
#include <vector>
#include <stdexcept>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& matrixA,
                                                     const std::vector<std::vector<double>>& matrixB) {
        size_t rowsA = matrixA.size();
        if (rowsA == 0) throw std::invalid_argument("Matrix A has no rows");
        size_t colsA = matrixA[0].size();
        size_t rowsB = matrixB.size();
        if (rowsB == 0) throw std::invalid_argument("Matrix B has no rows");
        size_t colsB = matrixB[0].size();

        for (size_t i = 1; i < rowsA; ++i) {
            if (matrixA[i].size() != colsA) {
                throw std::invalid_argument("Matrix A rows have inconsistent sizes");
            }
        }
        for (size_t i = 1; i < rowsB; ++i) {
            if (matrixB[i].size() != colsB) {
                throw std::invalid_argument("Matrix B rows have inconsistent sizes");
            }
        }

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
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
        std::vector<std::vector<double>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<double>> B = {{7, 8}, {9, 10}, {11, 12}};

        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        MatrixMultiplier::printMatrix(B);

        auto result = MatrixMultiplier::multiply(A, B);

        std::cout << "Result of A * B:" << std::endl;
        MatrixMultiplier::printMatrix(result);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}