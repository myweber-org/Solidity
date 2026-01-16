
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_matrix(size_t rows, size_t cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {

    size_t rows_A = A.size();
    size_t cols_A = A[0].size();
    size_t cols_B = B[0].size();
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));

    #pragma omp parallel for collapse(2)
    for (size_t i = 0; i < rows_A; ++i) {
        for (size_t j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

int main() {
    const size_t N = 500;
    auto matrix_A = generate_matrix(N, N);
    auto matrix_B = generate_matrix(N, N);

    auto start = std::chrono::high_resolution_clock::now();
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Parallel multiplication of " << N << "x" << N << " matrices took "
              << elapsed.count() << " seconds." << std::endl;

    return 0;
}
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
            C[i][j] = 0.0;
            for (int k = 0; k < size; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    std::vector<std::vector<double>> A, B, C;
    
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::cout << "Initializing matrices..." << std::endl;
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    C.resize(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));
    
    std::cout << "Performing matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    
    std::cout << "Matrix multiplication completed in " << elapsed << " seconds" << std::endl;
    
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 50) {
        for (int j = 0; j < MATRIX_SIZE; j += 50) {
            checksum += C[i][j];
        }
    }
    std::cout << "Checksum: " << checksum << std::endl;
    
    return 0;
}