
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    for (int i = 0; i < size; ++i) {
        matrix[i].resize(size);
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatricesSequential(const std::vector<std::vector<double>>& A,
                                const std::vector<std::vector<double>>& B,
                                std::vector<std::vector<double>>& C,
                                int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            C[i][j] = 0.0;
            for (int k = 0; k < size; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void multiplyMatricesParallel(const std::vector<std::vector<double>>& A,
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

bool verifyResults(const std::vector<std::vector<double>>& C1,
                   const std::vector<std::vector<double>>& C2,
                   int size,
                   double tolerance = 1e-9) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (std::abs(C1[i][j] - C2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int MATRIX_SIZE = 500;
    const int NUM_THREADS = 4;
    
    omp_set_num_threads(NUM_THREADS);
    
    std::vector<std::vector<double>> A(MATRIX_SIZE);
    std::vector<std::vector<double>> B(MATRIX_SIZE);
    std::vector<std::vector<double>> C_seq(MATRIX_SIZE);
    std::vector<std::vector<double>> C_par(MATRIX_SIZE);
    
    srand(42);
    
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    multiplyMatricesSequential(A, B, C_seq, MATRIX_SIZE);
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    multiplyMatricesParallel(A, B, C_par, MATRIX_SIZE);
    end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    bool resultsMatch = verifyResults(C_seq, C_par, MATRIX_SIZE);
    
    std::cout << "Matrix size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    std::cout << "Number of threads: " << NUM_THREADS << std::endl;
    std::cout << "Sequential execution time: " << seq_duration.count() << " ms" << std::endl;
    std::cout << "Parallel execution time: " << par_duration.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(seq_duration.count()) / par_duration.count() << std::endl;
    std::cout << "Results verification: " << (resultsMatch ? "PASSED" : "FAILED") << std::endl;
    
    return 0;
}