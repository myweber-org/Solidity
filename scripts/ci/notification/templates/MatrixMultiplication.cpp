
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

const int MATRIX_SIZE = 512;

void initializeMatrix(std::vector<std::vector<double>>& matrix) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            double sum = 0.0;
            #pragma omp simd reduction(+:sum)
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::vector<std::vector<double>> matrixA(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));
    std::vector<std::vector<double>> matrixB(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));
    std::vector<std::vector<double>> result(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));
    
    initializeMatrix(matrixA);
    initializeMatrix(matrixB);
    
    double startTime = omp_get_wtime();
    multiplyMatrices(matrixA, matrixB, result);
    double endTime = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed in " << (endTime - startTime) << " seconds" << std::endl;
    
    return 0;
}