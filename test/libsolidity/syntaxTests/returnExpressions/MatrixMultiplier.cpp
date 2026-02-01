
#include <iostream>
#include <vector>
#include <omp.h>
#include <cstdlib>
#include <ctime>

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
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
            }
        }
    }
};

int main() {
    const size_t N = 512;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::vector<std::vector<double>> matrixA(N, std::vector<double>(N));
    std::vector<std::vector<double>> matrixB(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixA);
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixB);
    
    double startTime = omp_get_wtime();
    auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << std::endl;
    std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
    
    double checksum = 0.0;
    for (size_t i = 0; i < 10; ++i) {
        checksum += result[i][i];
    }
    std::cout << "Diagonal checksum (first 10 elements): " << checksum << std::endl;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    int size;

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrixA.resize(n, std::vector<double>(n));
        matrixB.resize(n, std::vector<double>(n));
        result.resize(n, std::vector<double>(n, 0.0));
        initializeMatrices();
    }

    void initializeMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayResult(int limit = 5) {
        int displaySize = std::min(size, limit);
        std::cout << "First " << displaySize << "x" << displaySize << " elements of result matrix:\n";
        for (int i = 0; i < displaySize; ++i) {
            for (int j = 0; j < displaySize; ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() {
        double totalSum = 0.0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                totalSum += result[i][j];
            }
        }
        return totalSum;
    }
};

int main() {
    const int MATRIX_SIZE = 500;
    
    std::cout << "Initializing parallel matrix multiplier with size " << MATRIX_SIZE << "x" << MATRIX_SIZE << "\n";
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    double startTime = omp_get_wtime();
    multiplier.multiply();
    double endTime = omp_get_wtime();
    
    std::cout << "Parallel matrix multiplication completed in " << (endTime - startTime) << " seconds\n";
    
    multiplier.displayResult();
    
    double verificationSum = multiplier.verifyMultiplication();
    std::cout << "Verification sum of all matrix elements: " << verificationSum << "\n";
    
    return 0;
}