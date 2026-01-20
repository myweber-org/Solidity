
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
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

bool compare_matrices(const std::vector<std::vector<double>>& A,
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
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << std::endl;
    
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    std::cout << "Starting parallel matrix multiplication..." << std::endl;
    double start_parallel = omp_get_wtime();
    auto result_parallel = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_parallel = omp_get_wtime();
    
    std::cout << "Starting sequential matrix multiplication..." << std::endl;
    double start_sequential = omp_get_wtime();
    auto result_sequential = multiply_matrices_sequential(matrix_A, matrix_B);
    double end_sequential = omp_get_wtime();
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Parallel execution time: " << (end_parallel - start_parallel) << " seconds" << std::endl;
    std::cout << "Sequential execution time: " << (end_sequential - start_sequential) << " seconds" << std::endl;
    std::cout << "Speedup: " << (end_sequential - start_sequential) / (end_parallel - start_parallel) << "x" << std::endl;
    
    if (compare_matrices(result_parallel, result_sequential)) {
        std::cout << "Results match!" << std::endl;
    } else {
        std::cout << "Results do not match!" << std::endl;
    }
    
    return 0;
}