
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
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

int main() {
    const int N = 500;
    srand(42);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    auto genTime = std::chrono::high_resolution_clock::now();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    auto multTime = std::chrono::high_resolution_clock::now();
    
    auto genDuration = std::chrono::duration_cast<std::chrono::milliseconds>(genTime - start);
    auto multDuration = std::chrono::duration_cast<std::chrono::milliseconds>(multTime - genTime);
    
    std::cout << "Matrix generation time: " << genDuration.count() << " ms" << std::endl;
    std::cout << "Matrix multiplication time: " << multDuration.count() << " ms" << std::endl;
    std::cout << "Total execution time: " << (genDuration + multDuration).count() << " ms" << std::endl;
    
    double sampleValue = result[N/2][N/2];
    std::cout << "Sample value at center: " << sampleValue << std::endl;
    
    return 0;
}