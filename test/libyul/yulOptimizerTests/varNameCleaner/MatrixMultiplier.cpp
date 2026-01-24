
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
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

bool verifyResults(const std::vector<std::vector<double>>& A,
                   const std::vector<std::vector<double>>& B,
                   double tolerance = 1e-6) {
    
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (std::abs(A[i][j] - B[i][j]) > tolerance) {
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
    
    std::cout << "Starting parallel matrix multiplication..." << std::endl;
    auto startParallel = std::chrono::high_resolution_clock::now();
    auto resultParallel = multiplyMatricesParallel(A, B);
    auto endParallel = std::chrono::high_resolution_clock::now();
    auto parallelDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endParallel - startParallel);
    
    std::cout << "Starting sequential matrix multiplication..." << std::endl;
    auto startSequential = std::chrono::high_resolution_clock::now();
    auto resultSequential = multiplyMatricesSequential(A, B);
    auto endSequential = std::chrono::high_resolution_clock::now();
    auto sequentialDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endSequential - startSequential);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Parallel execution time: " << parallelDuration.count() << " ms" << std::endl;
    std::cout << "Sequential execution time: " << sequentialDuration.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(sequentialDuration.count()) / parallelDuration.count() << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (verifyResults(resultParallel, resultSequential)) {
        std::cout << "Results match! Parallel computation is correct." << std::endl;
    } else {
        std::cout << "ERROR: Results do not match!" << std::endl;
    }
    
    std::cout << "\nResult matrix dimensions: " << resultParallel.size() << "x" << resultParallel[0].size() << std::endl;
    
    return 0;
}