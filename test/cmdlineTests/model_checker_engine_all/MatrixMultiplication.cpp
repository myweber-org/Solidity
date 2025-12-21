
#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
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
    std::vector<std::vector<int>> matrixA = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    std::vector<std::vector<int>> matrixB = {
        {9, 8, 7},
        {6, 5, 4},
        {3, 2, 1}
    };

    std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);

    std::cout << "Matrix A:" << std::endl;
    printMatrix(matrixA);
    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(matrixB);
    std::cout << "\nProduct of A and B:" << std::endl;
    printMatrix(product);

    return 0;
}