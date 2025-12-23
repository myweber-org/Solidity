
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_matrix(size_t rows, size_t cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(i + j);
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    size_t rows_A = A.size();
    size_t cols_A = A[0].size();
    size_t cols_B = B[0].size();
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (size_t i = 0; i < rows_A; ++i) {
        for (size_t j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

int main() {
    const size_t N = 500;
    
    auto matrix_A = generate_matrix(N, N);
    auto matrix_B = generate_matrix(N, N);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    auto result_matrix = multiply_matrices_parallel(matrix_A, matrix_B);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
    
    return 0;
}