
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

void print_matrix_summary(const std::vector<std::vector<double>>& matrix) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    std::cout << "Matrix " << rows << "x" << cols << " - ";
    std::cout << "First element: " << matrix[0][0] << ", ";
    std::cout << "Last element: " << matrix[rows-1][cols-1] << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices..." << std::endl;
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    std::cout << "Multiplying matrices using OpenMP parallelization..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result_matrix = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;
    
    std::cout << "Matrix multiplication completed in " << elapsed_time << " seconds" << std::endl;
    std::cout << "Performance: " << (2.0 * N * M * P) / (elapsed_time * 1e9) << " GFLOPS" << std::endl;
    
    print_matrix_summary(result_matrix);
    
    return 0;
}