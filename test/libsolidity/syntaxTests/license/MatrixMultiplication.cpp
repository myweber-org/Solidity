
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
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

void print_matrix_summary(const std::vector<std::vector<double>>& matrix, const std::string& name) {
    std::cout << name << " summary (first 3x3):" << std::endl;
    int rows_to_print = std::min(3, (int)matrix.size());
    int cols_to_print = std::min(3, (int)matrix[0].size());
    
    for (int i = 0; i < rows_to_print; ++i) {
        for (int j = 0; j < cols_to_print; ++j) {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "Matrix size: " << matrix.size() << "x" << matrix[0].size() << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices..." << std::endl;
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_time = omp_get_wtime();
    
    std::cout << "Parallel multiplication completed in " << (end_time - start_time) 
              << " seconds" << std::endl;
    
    print_matrix_summary(matrix_A, "Matrix A");
    print_matrix_summary(matrix_B, "Matrix B");
    print_matrix_summary(result, "Result Matrix");
    
    return 0;
}