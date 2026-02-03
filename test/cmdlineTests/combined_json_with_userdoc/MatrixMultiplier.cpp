
#include <iostream>
#include <vector>
#include <chrono>
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

int main() {
    const int SIZE = 500;
    
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(matrixA, matrixB);
    auto end = std::chrono::high_resolution_clock::now();
    auto durationSeq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(matrixA, matrixB);
    end = std::chrono::high_resolution_clock::now();
    auto durationPar = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential multiplication time: " << durationSeq.count() << " ms" << std::endl;
    std::cout << "Parallel multiplication time: " << durationPar.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(durationSeq.count()) / durationPar.count() << std::endl;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t colsB = B[0].size();
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        return result;
    }
    
    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
        #pragma omp parallel for
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[i].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
            }
        }
    }
    
    static bool validateMultiplication(const std::vector<std::vector<double>>& A,
                                       const std::vector<std::vector<double>>& B,
                                       const std::vector<std::vector<double>>& C) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t colsB = B[0].size();
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double expected = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    expected += A[i][k] * B[k][j];
                }
                if (std::abs(C[i][j] - expected) > 1e-6) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const size_t N = 512;
    std::vector<std::vector<double>> matrixA(N, std::vector<double>(N));
    std::vector<std::vector<double>> matrixB(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixA);
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixB);
    
    double startTime = omp_get_wtime();
    auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed in " << (endTime - startTime) << " seconds\n";
    
    if (ParallelMatrixMultiplier::validateMultiplication(matrixA, matrixB, result)) {
        std::cout << "Result validation: PASSED\n";
    } else {
        std::cout << "Result validation: FAILED\n";
    }
    
    return 0;
}