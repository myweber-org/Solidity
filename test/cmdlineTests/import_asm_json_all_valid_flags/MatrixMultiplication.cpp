
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
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
    srand(42);
    
    auto A = generate_random_matrix(SIZE, SIZE);
    auto B = generate_random_matrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto C_seq = multiply_matrices_sequential(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto C_par = multiply_matrices_parallel(A, B);
    end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential multiplication time: " << seq_duration.count() << " ms\n";
    std::cout << "Parallel multiplication time: " << par_duration.count() << " ms\n";
    std::cout << "Speedup: " << static_cast<double>(seq_duration.count()) / par_duration.count() << "x\n";
    
    return 0;
}