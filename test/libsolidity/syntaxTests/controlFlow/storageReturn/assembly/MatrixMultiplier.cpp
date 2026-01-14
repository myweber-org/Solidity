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