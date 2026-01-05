
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
    const int NUM_THREADS = 4;

    omp_set_num_threads(NUM_THREADS);

    std::vector<std::vector<double>> A(MATRIX_SIZE);
    std::vector<std::vector<double>> B(MATRIX_SIZE);
    std::vector<std::vector<double>> C(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE, 0.0));

    srand(static_cast<unsigned>(time(nullptr)));

    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);

    auto start = std::chrono::high_resolution_clock::now();

    multiplyMatrices(A, B, C, MATRIX_SIZE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Matrix multiplication completed with size " << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
    std::cout << "Using " << NUM_THREADS << " OpenMP threads" << std::endl;

    return 0;
}