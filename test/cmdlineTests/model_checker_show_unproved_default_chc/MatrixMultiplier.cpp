
#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    size_t rowsA, colsA, rowsB, colsB;

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));
        
        initializeMatrices();
    }

    void initializeMatrices() {
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
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Sequential multiplication time: " << duration.count() << " ms" << std::endl;
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
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Parallel multiplication time: " << duration.count() << " ms" << std::endl;
    }

    void verifyResult(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        bool correct = true;
        
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    correct = false;
                    std::cout << "Mismatch at position (" << i << "," << j << ")" << std::endl;
                }
            }
        }
        
        if (correct) {
            std::cout << "Result verification: PASSED" << std::endl;
        } else {
            std::cout << "Result verification: FAILED" << std::endl;
        }
    }

    void displayPerformanceComparison() {
        std::cout << "\nMatrix dimensions: A[" << rowsA << "x" << colsA 
                  << "] * B[" << rowsB << "x" << colsB << "]" << std::endl;
        std::cout << "Available threads: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    const size_t SIZE = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        multiplier.displayPerformanceComparison();
        
        std::cout << "\nPerforming sequential multiplication..." << std::endl;
        multiplier.multiplySequential();
        
        std::cout << "\nPerforming parallel multiplication..." << std::endl;
        multiplier.multiplyParallel();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}