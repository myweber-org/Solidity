
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
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

bool verify_results(const std::vector<std::vector<double>>& C1,
                    const std::vector<std::vector<double>>& C2,
                    double tolerance = 1e-9) {
    if (C1.size() != C2.size() || C1[0].size() != C2[0].size()) {
        return false;
    }

    for (size_t i = 0; i < C1.size(); ++i) {
        for (size_t j = 0; j < C1[0].size(); ++j) {
            if (std::abs(C1[i][j] - C2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int N = 500;
    const int M = 500;
    const int P = 500;

    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << std::endl;
    auto A = generate_random_matrix(N, M);
    auto B = generate_random_matrix(M, P);

    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto C_seq = multiply_matrices_sequential(A, B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_duration = end_seq - start_seq;
    std::cout << "Sequential execution time: " << seq_duration.count() << " seconds" << std::endl;

    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
    auto C_par = multiply_matrices_parallel(A, B);
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_duration = end_par - start_par;
    std::cout << "Parallel execution time: " << par_duration.count() << " seconds" << std::endl;

    std::cout << "Speedup: " << seq_duration.count() / par_duration.count() << "x" << std::endl;

    std::cout << "Verifying results..." << std::endl;
    if (verify_results(C_seq, C_par)) {
        std::cout << "Results match!" << std::endl;
    } else {
        std::cout << "Error: Results do not match!" << std::endl;
    }

    return 0;
}