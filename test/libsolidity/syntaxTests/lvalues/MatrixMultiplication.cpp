
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
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

bool verify_matrices(const std::vector<std::vector<double>>& A,
                     const std::vector<std::vector<double>>& B,
                     double tolerance = 1e-6) {
    
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (std::abs(A[i][j] - B[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int N = 500;
    const int M = 500;
    const int K = 500;
    
    srand(42);
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << K << std::endl;
    
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, K);
    
    std::cout << "Starting sequential matrix multiplication..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    
    std::cout << "Starting parallel matrix multiplication..." << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Sequential time: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "Parallel time: " << duration_par.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(duration_seq.count()) / duration_par.count() << "x" << std::endl;
    
    if (verify_matrices(result_seq, result_par)) {
        std::cout << "Verification: Matrices match!" << std::endl;
    } else {
        std::cout << "Verification: Matrices do NOT match!" << std::endl;
    }
    
    return 0;
}