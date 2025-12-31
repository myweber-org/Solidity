
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
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
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
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void verify_results(const std::vector<std::vector<double>>& seq_result,
                   const std::vector<std::vector<double>>& par_result) {
    double tolerance = 1e-10;
    bool correct = true;
    
    for (size_t i = 0; i < seq_result.size(); ++i) {
        for (size_t j = 0; j < seq_result[0].size(); ++j) {
            if (std::abs(seq_result[i][j] - par_result[i][j]) > tolerance) {
                correct = false;
                break;
            }
        }
        if (!correct) break;
    }
    
    if (correct) {
        std::cout << "Results verification: PASSED" << std::endl;
    } else {
        std::cout << "Results verification: FAILED" << std::endl;
    }
}

int main() {
    const int N = 500;
    
    std::cout << "Matrix multiplication benchmark (N=" << N << ")" << std::endl;
    
    auto A = generate_matrix(N, N);
    auto B = generate_matrix(N, N);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto seq_result = multiply_matrices_sequential(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration<double>(end - start).count();
    
    start = std::chrono::high_resolution_clock::now();
    auto par_result = multiply_matrices_parallel(A, B);
    end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration<double>(end - start).count();
    
    verify_results(seq_result, par_result);
    
    std::cout << "Sequential execution time: " << seq_duration << " seconds" << std::endl;
    std::cout << "Parallel execution time: " << par_duration << " seconds" << std::endl;
    std::cout << "Speedup: " << seq_duration / par_duration << "x" << std::endl;
    
    return 0;
}