
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(i + j);
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

std::vector<std::vector<double>> multiply_matrices_sequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
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
    const int SIZE = 500;
    
    auto A = generate_matrix(SIZE, SIZE);
    auto B = generate_matrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto C_parallel = multiply_matrices_parallel(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallel_time = end - start;
    
    start = std::chrono::high_resolution_clock::now();
    auto C_sequential = multiply_matrices_sequential(A, B);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> sequential_time = end - start;
    
    std::cout << "Matrix size: " << SIZE << "x" << SIZE << std::endl;
    std::cout << "Parallel execution time: " << parallel_time.count() << " seconds" << std::endl;
    std::cout << "Sequential execution time: " << sequential_time.count() << " seconds" << std::endl;
    std::cout << "Speedup: " << sequential_time.count() / parallel_time.count() << "x" << std::endl;
    
    bool results_match = true;
    for (int i = 0; i < SIZE && results_match; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (std::abs(C_parallel[i][j] - C_sequential[i][j]) > 1e-6) {
                results_match = false;
                break;
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