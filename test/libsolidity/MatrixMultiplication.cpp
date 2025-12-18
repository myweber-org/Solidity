
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A,
                                                  const std::vector<std::vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

void printMatrix(const std::vector<std::vector<double>>& matrix) {
    for (const auto& row : matrix) {
        for (double val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << "..." << std::endl;
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    auto result = multiplyMatrices(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Multiplication completed in " << (endTime - startTime) << " seconds." << std::endl;
    
    if (N <= 5 && M <= 5 && P <= 5) {
        std::cout << "\nMatrix A:" << std::endl;
        printMatrix(matrixA);
        std::cout << "\nMatrix B:" << std::endl;
        printMatrix(matrixB);
        std::cout << "\nResult Matrix:" << std::endl;
        printMatrix(result);
    } else {
        std::cout << "Result matrix is too large to display. Showing first 3x3 block:" << std::endl;
        for (int i = 0; i < std::min(3, N); ++i) {
            for (int j = 0; j < std::min(3, P); ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
    
    return 0;
}