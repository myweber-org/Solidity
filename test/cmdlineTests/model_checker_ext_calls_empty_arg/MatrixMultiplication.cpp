
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_matrix(size_t n, double value) {
    std::vector<std::vector<double>> matrix(n, std::vector<double>(n));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            matrix[i][j] = value;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    size_t n = A.size();
    std::vector<std::vector<double>> C(n, std::vector<double>(n, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    return C;
}

int main() {
    const size_t MATRIX_SIZE = 500;
    
    auto matrix_A = generate_matrix(MATRIX_SIZE, 2.5);
    auto matrix_B = generate_matrix(MATRIX_SIZE, 1.5);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    auto result_matrix = multiply_matrices_parallel(matrix_A, matrix_B);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    std::cout << "Matrix multiplication completed for size " 
              << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << duration.count() 
              << " milliseconds" << std::endl;
    
    return 0;
}