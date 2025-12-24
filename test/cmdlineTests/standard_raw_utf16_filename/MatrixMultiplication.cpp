
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
    int rows_B = B.size();
    int cols_B = B[0].size();
    
    if (cols_A != rows_B) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
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

void print_matrix_dimensions(const std::vector<std::vector<double>>& matrix, const std::string& name) {
    std::cout << name << " dimensions: " << matrix.size() << " x " << matrix[0].size() << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices..." << std::endl;
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    print_matrix_dimensions(matrix_A, "Matrix A");
    print_matrix_dimensions(matrix_B, "Matrix B");
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result_matrix = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    double execution_time = end_time - start_time;
    
    print_matrix_dimensions(result_matrix, "Result matrix");
    std::cout << "Execution time: " << execution_time << " seconds" << std::endl;
    
    if (result_matrix.size() > 0 && result_matrix[0].size() > 0) {
        std::cout << "Sample element [0][0]: " << result_matrix[0][0] << std::endl;
    }
    
    return 0;
}