#include <iostream>
#include <array>

const int SIZE = 3;

using Matrix = std::array<std::array<int, SIZE>, SIZE>;

void multiplyMatrices(const Matrix& a, const Matrix& b, Matrix& result) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void printMatrix(const Matrix& m) {
    for (const auto& row : m) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    Matrix matA = {{
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    }};

    Matrix matB = {{
        {9, 8, 7},
        {6, 5, 4},
        {3, 2, 1}
    }};

    Matrix result;

    multiplyMatrices(matA, matB, result);

    std::cout << "Matrix A:\n";
    printMatrix(matA);

    std::cout << "\nMatrix B:\n";
    printMatrix(matB);

    std::cout << "\nResult (A * B):\n";
    printMatrix(result);

    return 0;
}