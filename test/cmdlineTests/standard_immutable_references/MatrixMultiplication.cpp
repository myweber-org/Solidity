
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
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    return C;
}

int main() {
    const int SIZE = 500;
    
    auto matrix_A = generate_matrix(SIZE, SIZE);
    auto matrix_B = generate_matrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Matrix multiplication completed for " << SIZE << "x" << SIZE << " matrices." << std::endl;
    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
    
    double checksum = 0.0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            checksum += result[i][j];
        }
    }
    std::cout << "Checksum of first 10x10 elements: " << checksum << std::endl;
    
    return 0;
}