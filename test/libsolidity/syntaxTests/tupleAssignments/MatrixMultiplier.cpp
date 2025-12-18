#include <iostream>
#include <vector>
#include <stdexcept>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t rowsA = A.size();
        if (rowsA == 0) throw std::invalid_argument("Matrix A has zero rows.");
        size_t colsA = A[0].size();
        size_t rowsB = B.size();
        if (rowsB == 0) throw std::invalid_argument("Matrix B has zero rows.");
        size_t colsB = B[0].size();

        for (size_t i = 0; i < rowsA; ++i) {
            if (A[i].size() != colsA) {
                throw std::invalid_argument("Matrix A has inconsistent column count.");
            }
        }
        for (size_t i = 0; i < rowsB; ++i) {
            if (B[i].size() != colsB) {
                throw std::invalid_argument("Matrix B has inconsistent column count.");
            }
        }

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication.");
        }

        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
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
        std::vector<std::vector<double>> A = {{1.0, 2.0, 3.0},
                                              {4.0, 5.0, 6.0}};
        std::vector<std::vector<double>> B = {{7.0, 8.0},
                                              {9.0, 10.0},
                                              {11.0, 12.0}};

        std::vector<std::vector<double>> C = MatrixMultiplier::multiply(A, B);

        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        MatrixMultiplier::printMatrix(B);
        std::cout << "Result matrix C (A * B):" << std::endl;
        MatrixMultiplier::printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}