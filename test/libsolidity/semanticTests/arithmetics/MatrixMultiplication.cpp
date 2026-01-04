
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

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    std::cout << "Generating random " << SIZE << "x" << SIZE << " matrices..." << std::endl;
    auto matrixA = generate_random_matrix(SIZE, SIZE);
    auto matrixB = generate_random_matrix(SIZE, SIZE);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrixA, matrixB);
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    
    std::cout << "Multiplication completed in " << elapsed << " seconds" << std::endl;
    std::cout << "Result matrix dimensions: " << result.size() << "x" << result[0].size() << std::endl;
    
    double sample_sum = 0.0;
    for (int i = 0; i < 10 && i < SIZE; ++i) {
        for (int j = 0; j < 10 && j < SIZE; ++j) {
            sample_sum += result[i][j];
        }
    }
    std::cout << "Sample sum of first 10x10 elements: " << sample_sum << std::endl;
    
    return 0;
}