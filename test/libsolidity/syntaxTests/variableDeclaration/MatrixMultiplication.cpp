
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
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    if (n != B.size()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
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

std::vector<std::vector<double>> multiply_matrices_sequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    if (n != B.size()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
    std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));
    
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
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    std::cout << "Generating random " << SIZE << "x" << SIZE << " matrices..." << std::endl;
    auto A = generate_random_matrix(SIZE, SIZE);
    auto B = generate_random_matrix(SIZE, SIZE);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_parallel = omp_get_wtime();
    auto C_parallel = multiply_matrices_parallel(A, B);
    double end_parallel = omp_get_wtime();
    
    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    double start_sequential = omp_get_wtime();
    auto C_sequential = multiply_matrices_sequential(A, B);
    double end_sequential = omp_get_wtime();
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Parallel execution time: " << (end_parallel - start_parallel) << " seconds" << std::endl;
    std::cout << "Sequential execution time: " << (end_sequential - start_sequential) << " seconds" << std::endl;
    std::cout << "Speedup factor: " << (end_sequential - start_sequential) / (end_parallel - start_parallel) << std::endl;
    
    bool results_match = true;
    for (int i = 0; i < SIZE && results_match; ++i) {
        for (int j = 0; j < SIZE && results_match; ++j) {
            if (std::abs(C_parallel[i][j] - C_sequential[i][j]) > 1e-6) {
                results_match = false;
            }
        }
    }
    
    if (results_match) {
        std::cout << "Verification: Parallel and sequential results match!" << std::endl;
    } else {
        std::cout << "Verification: Results do not match!" << std::endl;
    }
    
    return 0;
}