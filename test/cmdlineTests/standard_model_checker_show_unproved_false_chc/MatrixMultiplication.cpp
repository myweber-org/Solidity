
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
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

bool verify_results(const std::vector<std::vector<double>>& A,
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
    std::cout << "Generating random " << N << "x" << N << " matrices..." << std::endl;
    
    auto matrix_A = generate_random_matrix(N, N);
    auto matrix_B = generate_random_matrix(N, N);
    
    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    double start_seq = omp_get_wtime();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    double end_seq = omp_get_wtime();
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_par = omp_get_wtime();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_par = omp_get_wtime();
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Sequential time: " << (end_seq - start_seq) << " seconds" << std::endl;
    std::cout << "Parallel time: " << (end_par - start_par) << " seconds" << std::endl;
    std::cout << "Speedup: " << (end_seq - start_seq) / (end_par - start_par) << "x" << std::endl;
    
    if (verify_results(result_seq, result_par)) {
        std::cout << "Verification: Results match!" << std::endl;
    } else {
        std::cout << "Verification: Results do not match!" << std::endl;
    }
    
    return 0;
}