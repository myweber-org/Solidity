
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
    const int ROWS_A = 500;
    const int COLS_A = 500;
    const int COLS_B = 500;
    
    srand(42);
    
    std::cout << "Generating random matrices..." << std::endl;
    auto A = generateRandomMatrix(ROWS_A, COLS_A);
    auto B = generateRandomMatrix(COLS_A, COLS_B);
    
    std::cout << "Matrix dimensions: " << ROWS_A << "x" << COLS_A 
              << " * " << COLS_A << "x" << COLS_B << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    auto seqDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential multiplication time: " << seqDuration.count() << " ms" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(A, B);
    end = std::chrono::high_resolution_clock::now();
    auto parDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parallel multiplication time: " << parDuration.count() << " ms" << std::endl;
    
    std::cout << "Speedup: " << static_cast<double>(seqDuration.count()) / parDuration.count() 
              << "x" << std::endl;
    
    if (verifyResults(resultSeq, resultPar)) {
        std::cout << "Results verification: PASSED" << std::endl;
    } else {
        std::cout << "Results verification: FAILED" << std::endl;
    }
    
    return 0;
}