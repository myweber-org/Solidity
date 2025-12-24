
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
    size_t rowsA, colsA, rowsB, colsB;

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, size_t rows, size_t cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        srand(static_cast<unsigned>(time(nullptr)));
        initializeRandomMatrix(matrixA, rowsA, colsA);
        initializeRandomMatrix(matrixB, rowsB, colsB);
        result.resize(rowsA, std::vector<double>(colsB, 0.0));
    }

    void multiplySequential() {
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyMultiplication() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        double tolerance = 1e-9;
        bool correct = true;
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > tolerance) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void displayResult(size_t maxRows = 5, size_t maxCols = 5) {
        std::cout << "Result matrix (first " << maxRows << "x" << maxCols << " elements):" << std::endl;
        for (size_t i = 0; i < std::min(rowsA, maxRows); ++i) {
            for (size_t j = 0; j < std::min(colsB, maxCols); ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    double benchmarkMultiplication(bool parallel = true) {
        double startTime = omp_get_wtime();
        
        if (parallel) {
            multiplyParallel();
        } else {
            multiplySequential();
        }
        
        double endTime = omp_get_wtime();
        return endTime - startTime;
    }
};

int main() {
    const size_t SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix dimensions: " << SIZE << "x" << SIZE << std::endl;
        std::cout << "Number of threads: " << omp_get_max_threads() << std::endl;
        
        double seqTime = multiplier.benchmarkMultiplication(false);
        std::cout << "Sequential execution time: " << seqTime << " seconds" << std::endl;
        
        double parTime = multiplier.benchmarkMultiplication(true);
        std::cout << "Parallel execution time: " << parTime << " seconds" << std::endl;
        
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
        
        multiplier.verifyMultiplication();
        multiplier.displayResult();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}