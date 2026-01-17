
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
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
    const int N = 500;
    auto matrix_A = generate_random_matrix(N, N);
    auto matrix_B = generate_random_matrix(N, N);

    auto start = std::chrono::high_resolution_clock::now();
    auto result_parallel = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallel_duration = end - start;
    std::cout << "Parallel multiplication time: " << parallel_duration.count() << " seconds\n";

    start = std::chrono::high_resolution_clock::now();
    auto result_sequential = multiply_matrices_sequential(matrix_A, matrix_B);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> sequential_duration = end - start;
    std::cout << "Sequential multiplication time: " << sequential_duration.count() << " seconds\n";

    double speedup = sequential_duration.count() / parallel_duration.count();
    std::cout << "Speedup factor: " << speedup << "x\n";

    bool results_match = true;
    for (int i = 0; i < N && results_match; ++i) {
        for (int j = 0; j < N; ++j) {
            if (std::abs(result_parallel[i][j] - result_sequential[i][j]) > 1e-6) {
                results_match = false;
                break;
            }
        }
    }

    if (results_match) {
        std::cout << "Results verification: PASSED\n";
    } else {
        std::cout << "Results verification: FAILED\n";
    }

    return 0;
}