
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<int>> generateRandomMatrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

vector<vector<int>> multiplyMatrices(const vector<vector<int>>& A,
                                     const vector<vector<int>>& B) {
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    vector<vector<int>> result(m, vector<int>(p, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << val << "\t";
        }
        cout << endl;
    }
}

int main() {
    srand(time(0));
    
    const int ROWS_A = 500;
    const int COLS_A = 500;
    const int COLS_B = 500;
    
    cout << "Generating random matrices..." << endl;
    auto matrixA = generateRandomMatrix(ROWS_A, COLS_A);
    auto matrixB = generateRandomMatrix(COLS_A, COLS_B);
    
    cout << "Performing matrix multiplication..." << endl;
    double start_time = omp_get_wtime();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    
    double end_time = omp_get_wtime();
    
    cout << "Matrix multiplication completed in " 
         << (end_time - start_time) << " seconds" << endl;
    
    return 0;
}#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& matA,
                                               const std::vector<std::vector<int>>& matB) {
    int rowsA = matA.size();
    int colsA = matA[0].size();
    int colsB = matB[0].size();

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += matA[i][k] * matB[k][j];
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