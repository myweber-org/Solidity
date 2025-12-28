
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

bool verify_results(const std::vector<std::vector<double>>& mat1,
                    const std::vector<std::vector<double>>& mat2,
                    double tolerance = 1e-10) {
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (std::abs(mat1[i][j] - mat2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int SIZE = 500;
    
    std::cout << "Generating " << SIZE << "x" << SIZE << " matrices..." << std::endl;
    auto matrix_A = generate_matrix(SIZE, SIZE);
    auto matrix_B = generate_matrix(SIZE, SIZE);
    
    std::cout << "Performing sequential multiplication..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    
    std::cout << "Performing parallel multiplication..." << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Sequential time: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "Parallel time: " << duration_par.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(duration_seq.count()) / duration_par.count() << "x" << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (verify_results(result_seq, result_par)) {
        std::cout << "Results match! Parallel implementation is correct." << std::endl;
    } else {
        std::cout << "ERROR: Results do not match!" << std::endl;
    }
    
    return 0;
}