#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
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
    srand(static_cast<unsigned>(time(0)));
    
    const int N = 500;
    std::cout << "Generating " << N << "x" << N << " matrices..." << std::endl;
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    std::cout << "Starting parallel multiplication..." << std::endl;
    double startParallel = omp_get_wtime();
    auto resultParallel = multiplyMatricesParallel(matrixA, matrixB);
    double endParallel = omp_get_wtime();
    
    std::cout << "Starting sequential multiplication..." << std::endl;
    double startSequential = omp_get_wtime();
    auto resultSequential = multiplyMatricesSequential(matrixA, matrixB);
    double endSequential = omp_get_wtime();
    
    double parallelTime = endParallel - startParallel;
    double sequentialTime = endSequential - startSequential;
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
    std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
    std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
    
    if (verifyResults(resultParallel, resultSequential)) {
        std::cout << "Results verification: PASSED" << std::endl;
    } else {
        std::cout << "Results verification: FAILED" << std::endl;
    }
    
    return 0;
}