
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(i + j) / 100.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

int main() {
    const int N = 500;
    
    auto matrix_A = generate_matrix(N, N);
    auto matrix_B = generate_matrix(N, N);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parallel matrix multiplication of " << N << "x" << N << " matrices completed in "
              << duration.count() << " ms" << std::endl;
    
    double checksum = 0.0;
    for (int i = 0; i < std::min(10, N); ++i) {
        for (int j = 0; j < std::min(10, N); ++j) {
            checksum += result[i][j];
        }
    }
    std::cout << "Checksum of first 10x10 elements: " << checksum << std::endl;
    
    return 0;
}