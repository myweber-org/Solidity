
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

int main() {
    const int MATRIX_SIZE = 500;
    std::vector<std::vector<double>> A(MATRIX_SIZE), B(MATRIX_SIZE), C(MATRIX_SIZE);

    srand(static_cast<unsigned>(time(nullptr)));
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);

    auto start = std::chrono::high_resolution_clock::now();
    multiplyMatricesParallel(A, B, C, MATRIX_SIZE);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Parallel matrix multiplication completed in " << elapsed.count() << " seconds." << std::endl;

    return 0;
}