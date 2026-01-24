
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatricesParallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<double>> result(m, std::vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

std::vector<std::vector<double>> multiplyMatricesSequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<double>> result(m, std::vector<double>(p, 0.0));
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

bool verifyResults(const std::vector<std::vector<double>>& mat1,
                   const std::vector<std::vector<double>>& mat2,
                   double tolerance = 1e-6) {
    
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (std::abs(mat1[i][j] - mat2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int SIZE = 500;
    srand(42);
    
    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << std::endl;
    auto A = generateRandomMatrix(SIZE, SIZE);
    auto B = generateRandomMatrix(SIZE, SIZE);
    
    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto result_seq = multiplyMatricesSequential(A, B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    auto start_par = std::chrono::high_resolution_clock::now();
    auto result_par = multiplyMatricesParallel(A, B);
    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Sequential time: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "Parallel time: " << duration_par.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(duration_seq.count()) / duration_par.count() << "x" << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (verifyResults(result_seq, result_par)) {
        std::cout << "Results match! Parallel implementation is correct." << std::endl;
    } else {
        std::cout << "ERROR: Results do not match!" << std::endl;
    }
    
    return 0;
}