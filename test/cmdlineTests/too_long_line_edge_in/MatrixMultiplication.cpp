
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

const int N = 512;

void initializeMatrix(std::vector<std::vector<double>>& matrix) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<std::vector<double>> A(N, std::vector<double>(N));
    std::vector<std::vector<double>> B(N, std::vector<double>(N));
    std::vector<std::vector<double>> C(N, std::vector<double>(N, 0.0));

    initializeMatrix(A);
    initializeMatrix(B);

    double start_time = omp_get_wtime();
    multiplyMatrices(A, B, C);
    double end_time = omp_get_wtime();

    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;

    return 0;
}