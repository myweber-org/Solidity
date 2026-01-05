
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

std::vector<std::vector<double>> multiplyMatricesParallel(
    const std::vector<std::vector<double>>& A,
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

void printMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 3, int maxCols = 3) {
    int rows = std::min(static_cast<int>(matrix.size()), maxRows);
    int cols = std::min(static_cast<int>(matrix[0].size()), maxCols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "...\n";
    }
    std::cout << "...\n";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << "...\n";
    
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "First few elements of matrix A:\n";
    printMatrix(matrixA);
    
    std::cout << "\nFirst few elements of matrix B:\n";
    printMatrix(matrixB);
    
    std::cout << "\nPerforming parallel matrix multiplication...\n";
    
    double startTime = omp_get_wtime();
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "\nFirst few elements of result matrix:\n";
    printMatrix(result);
    
    std::cout << "\nParallel multiplication completed in " << (endTime - startTime) << " seconds\n";
    
    int numThreads = omp_get_max_threads();
    std::cout << "Using " << numThreads << " OpenMP threads\n";
    
    return 0;
}