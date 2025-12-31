
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

const int N = 512;

void initializeMatrix(std::vector<std::vector<double>>& matrix) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<std::vector<double>> A(N, std::vector<double>(N));
    std::vector<std::vector<double>> B(N, std::vector<double>(N));
    std::vector<std::vector<double>> C(N, std::vector<double>(N, 0.0));

    initializeMatrix(A);
    initializeMatrix(B);

    double start_time = omp_get_wtime();
    multiplyMatrices(A, B, C);
    double end_time = omp_get_wtime();

    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;

    return 0;
}
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

int main() {
    const int N = 500;
    srand(42);
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parallel matrix multiplication of " << N << "x" << N << " matrices completed in "
              << duration.count() << " ms" << std::endl;
    
    double checksum = 0.0;
    for (int i = 0; i < N; i += 50) {
        for (int j = 0; j < N; j += 50) {
            checksum += result[i][j];
        }
    }
    std::cout << "Result checksum: " << checksum << std::endl;
    
    return 0;
}