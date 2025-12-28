
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
    
    std::vector<std::vector<double>> result(m, std::vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
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
    
    const int SIZE = 500;
    
    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << std::endl;
    auto matrix_a = generate_random_matrix(SIZE, SIZE);
    auto matrix_b = generate_random_matrix(SIZE, SIZE);
    
    print_matrix_dimensions(matrix_a, "Matrix A");
    print_matrix_dimensions(matrix_b, "Matrix B");
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrix_a, matrix_b);
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    
    print_matrix_dimensions(result, "Result matrix");
    std::cout << "Computation time: " << elapsed << " seconds" << std::endl;
    
    double sample_value = result[SIZE/2][SIZE/2];
    std::cout << "Sample value at center: " << sample_value << std::endl;
    
    return 0;
}