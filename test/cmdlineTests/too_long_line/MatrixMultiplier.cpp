
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    size_t rowsA, colsA, rowsB, colsB;

    void initializeMatrices() {
        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsB; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        initializeMatrices();
    }

    void multiplySequential() {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Sequential multiplication time: " << elapsed.count() << " seconds\n";
    }

    void multiplyParallel() {
        auto start = std::chrono::high_resolution_clock::now();
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Parallel multiplication time: " << elapsed.count() << " seconds\n";
    }

    void verifyResult(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        bool correct = true;
        
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    correct = false;
                    std::cout << "Mismatch at position (" << i << "," << j << ")\n";
                }
            }
        }
        
        if (correct) {
            std::cout << "Result verification: PASSED\n";
        } else {
            std::cout << "Result verification: FAILED\n";
        }
    }

    void displayPerformanceStats() {
        size_t totalOperations = rowsA * colsB * colsA * 2;
        std::cout << "Total floating-point operations: " << totalOperations << "\n";
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " 
                  << rowsB << "x" << colsB << "\n";
    }
};

int main() {
    try {
        const size_t SIZE = 512;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        multiplier.displayPerformanceStats();
        std::cout << "\n";
        
        std::cout << "Running sequential multiplication...\n";
        multiplier.multiplySequential();
        
        std::cout << "\nRunning parallel multiplication...\n";
        multiplier.multiplyParallel();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}