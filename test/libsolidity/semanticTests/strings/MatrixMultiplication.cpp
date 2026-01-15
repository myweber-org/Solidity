
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

int main() {
    const int MATRIX_SIZE = 512;
    std::vector<std::vector<double>> A, B, C;
    
    srand(static_cast<unsigned>(time(nullptr)));
    
    double start = omp_get_wtime();
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    double init_time = omp_get_wtime() - start;
    
    start = omp_get_wtime();
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    double mult_time = omp_get_wtime() - start;
    
    std::cout << "Matrix size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    std::cout << "Initialization time: " << init_time << " seconds" << std::endl;
    std::cout << "Multiplication time: " << mult_time << " seconds" << std::endl;
    
    double total_ops = 2.0 * MATRIX_SIZE * MATRIX_SIZE * MATRIX_SIZE;
    double gflops = (total_ops / 1e9) / mult_time;
    std::cout << "Performance: " << gflops << " GFLOPS" << std::endl;
    
    return 0;
}