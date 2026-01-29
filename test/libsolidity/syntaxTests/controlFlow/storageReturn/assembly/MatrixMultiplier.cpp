#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A,
                                               const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
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
    std::vector<std::vector<int>> A = {{1, 2, 3},
                                       {4, 5, 6}};

    std::vector<std::vector<int>> B = {{7, 8},
                                       {9, 10},
                                       {11, 12}};

    std::vector<std::vector<int>> C = multiplyMatrices(A, B);

    std::cout << "Matrix A:" << std::endl;
    printMatrix(A);

    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(B);

    std::cout << "\nResult matrix C (A * B):" << std::endl;
    printMatrix(C);

    return 0;
}
#include <iostream>
#include <vector>
#include <stdexcept>

class MatrixMultiplier {
public:
    static std::vector<std::vector<int>> multiply(const std::vector<std::vector<int>>& A,
                                                  const std::vector<std::vector<int>>& B) {
        if (A.empty() || B.empty()) {
            throw std::invalid_argument("Input matrices cannot be empty.");
        }
        size_t aRows = A.size();
        size_t aCols = A[0].size();
        size_t bRows = B.size();
        size_t bCols = B[0].size();

        if (aCols != bRows) {
            throw std::invalid_argument("Matrix dimension mismatch for multiplication.");
        }

        std::vector<std::vector<int>> result(aRows, std::vector<int>(bCols, 0));

        for (size_t i = 0; i < aRows; ++i) {
            for (size_t j = 0; j < bCols; ++j) {
                for (size_t k = 0; k < aCols; ++k) {
                    result[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return result;
    }

    static void printMatrix(const std::vector<std::vector<int>>& matrix) {
        for (const auto& row : matrix) {
            for (int val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        std::vector<std::vector<int>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<int>> B = {{7, 8}, {9, 10}, {11, 12}};

        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        MatrixMultiplier::printMatrix(B);

        auto C = MatrixMultiplier::multiply(A, B);
        std::cout << "Result matrix C (A * B):" << std::endl;
        MatrixMultiplier::printMatrix(C);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A,
                                               const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
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
    std::vector<std::vector<int>> A = {{1, 2, 3},
                                       {4, 5, 6}};
    std::vector<std::vector<int>> B = {{7, 8},
                                       {9, 10},
                                       {11, 12}};

    std::vector<std::vector<int>> C = multiplyMatrices(A, B);

    std::cout << "Matrix A:" << std::endl;
    printMatrix(A);
    std::cout << "Matrix B:" << std::endl;
    printMatrix(B);
    std::cout << "Result A * B:" << std::endl;
    printMatrix(C);

    return 0;
}