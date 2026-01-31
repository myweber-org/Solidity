
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
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
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
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

bool verifyResults(const std::vector<std::vector<double>>& result1,
                   const std::vector<std::vector<double>>& result2,
                   double tolerance = 1e-6) {
    
    if (result1.size() != result2.size() || result1[0].size() != result2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < result1.size(); ++i) {
        for (size_t j = 0; j < result1[0].size(); ++j) {
            if (std::abs(result1[i][j] - result2[i][j]) > tolerance) {
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
    
    srand(42);
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << std::endl;
    auto A = generateRandomMatrix(N, M);
    auto B = generateRandomMatrix(M, P);
    
    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    auto durationSeq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Sequential execution time: " << durationSeq.count() << " ms" << std::endl;
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(A, B);
    end = std::chrono::high_resolution_clock::now();
    auto durationPar = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Parallel execution time: " << durationPar.count() << " ms" << std::endl;
    
    std::cout << "Speedup: " << static_cast<double>(durationSeq.count()) / durationPar.count() << "x" << std::endl;
    
    std::cout << "Verifying results..." << std::endl;
    if (verifyResults(resultSeq, resultPar)) {
        std::cout << "Results match!" << std::endl;
    } else {
        std::cout << "Results do not match!" << std::endl;
    }
    
    return 0;
}