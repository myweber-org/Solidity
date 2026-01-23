
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);
    
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
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
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << "...\n";
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "First few elements of matrix A:\n";
    printMatrix(matrixA);
    
    std::cout << "First few elements of matrix B:\n";
    printMatrix(matrixB);
    
    std::cout << "Performing matrix multiplication with OpenMP parallelization...\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "First few elements of result matrix:\n";
    printMatrix(result);
    
    std::cout << "Multiplication completed in " << duration.count() << " milliseconds.\n";
    
    double totalSum = 0.0;
    #pragma omp parallel for reduction(+:totalSum)
    for (size_t i = 0; i < result.size(); ++i) {
        for (size_t j = 0; j < result[0].size(); ++j) {
            totalSum += result[i][j];
        }
    }
    
    std::cout << "Sum of all elements in result matrix: " << totalSum << "\n";
    
    return 0;
}