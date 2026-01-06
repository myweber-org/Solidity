
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

int main() {
    const int N = 500;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::cout << "Generating random matrices of size " << N << "x" << N << "..." << std::endl;
    auto matrix_A = generate_random_matrix(N, N);
    auto matrix_B = generate_random_matrix(N, N);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    std::cout << "Multiplication completed in " << (end_time - start_time) << " seconds." << std::endl;
    
    double checksum = 0.0;
    for (int i = 0; i < N; i += 50) {
        checksum += result[i][i];
    }
    std::cout << "Diagonal checksum: " << checksum << std::endl;
    
    return 0;
}