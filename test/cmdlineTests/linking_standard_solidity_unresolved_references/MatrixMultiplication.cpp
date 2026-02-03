#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    C.resize(size, std::vector<double>(size, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            #pragma omp simd reduction(+:sum)
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void printMatrixStats(const std::vector<std::vector<double>>& matrix, int size) {
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            sum += matrix[i][j];
        }
    }
    std::cout << "Matrix element sum: " << sum << std::endl;
}

int main() {
    const int MATRIX_SIZE = 512;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::vector<std::vector<double>> A, B, C;
    
    double start_time = omp_get_wtime();
    
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    
    double end_time = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed for size " << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
    
    printMatrixStats(C, MATRIX_SIZE);
    
    return 0;
}