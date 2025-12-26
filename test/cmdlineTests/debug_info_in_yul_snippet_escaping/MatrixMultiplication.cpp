
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

std::vector<std::vector<double>> multiply_matrices_sequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
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
    const int SIZE = 500;
    
    auto matrix_A = generate_matrix(SIZE, SIZE);
    auto matrix_B = generate_matrix(SIZE, SIZE);
    
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    
    auto start_par = std::chrono::high_resolution_clock::now();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);
    
    std::cout << "Matrix size: " << SIZE << "x" << SIZE << std::endl;
    std::cout << "Sequential multiplication time: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "Parallel multiplication time: " << duration_par.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(duration_seq.count()) / duration_par.count() << std::endl;
    
    bool results_match = true;
    for (int i = 0; i < SIZE && results_match; ++i) {
        for (int j = 0; j < SIZE && results_match; ++j) {
            if (std::abs(result_seq[i][j] - result_par[i][j]) > 1e-6) {
                results_match = false;
            }
        }
    }
    
    if (results_match) {
        std::cout << "Results verification: PASSED" << std::endl;
    } else {
        std::cout << "Results verification: FAILED" << std::endl;
    }
    
    return 0;
}