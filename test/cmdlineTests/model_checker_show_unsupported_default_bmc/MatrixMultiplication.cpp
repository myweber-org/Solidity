
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
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

bool validateMultiplication(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B,
    const std::vector<std::vector<double>>& C) {
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            if (std::abs(sum - C[i][j]) > 1e-9) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 512;
    
    std::cout << "Generating random matrices of size " << N << "x" << N << "..." << std::endl;
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Multiplication completed in " << (endTime - startTime) << " seconds" << std::endl;
    
    std::cout << "Validating result..." << std::endl;
    if (validateMultiplication(matrixA, matrixB, result)) {
        std::cout << "Result validation successful!" << std::endl;
    } else {
        std::cout << "Result validation failed!" << std::endl;
    }
    
    return 0;
}