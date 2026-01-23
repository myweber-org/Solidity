
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<std::vector<double>> A, B, C;
    
    std::cout << "Initializing matrices..." << std::endl;
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    C.resize(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));

    std::cout << "Performing matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;

    std::cout << "Matrix multiplication completed." << std::endl;
    std::cout << "Matrix size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << elapsed << " seconds" << std::endl;

    return 0;
}
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

int main() {
    const int SIZE = 500;
    
    auto matrix_A = generate_matrix(SIZE, SIZE);
    auto matrix_B = generate_matrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    end = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential multiplication time: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "Parallel multiplication time: " << duration_par.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(duration_seq.count()) / duration_par.count() << std::endl;
    
    bool results_match = true;
    for (int i = 0; i < SIZE && results_match; ++i) {
        for (int j = 0; j < SIZE && results_match; ++j) {
            if (std::abs(result_seq[i][j] - result_par[i][j]) > 1e-6) {
                results_match = false;
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