
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

    void displayResult(size_t maxRows = 5, size_t maxCols = 5) const {
        std::cout << "Result matrix (first " << maxRows << "x" << maxCols << " elements):\n";
        for (size_t i = 0; i < std::min(rowsA, maxRows); ++i) {
            for (size_t j = 0; j < std::min(colsB, maxCols); ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication(const ParallelMatrixMultiplier& reference) const {
        double error = 0.0;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                error += std::abs(result[i][j] - reference.result[i][j]);
            }
        }
        return error;
    }
};

int main() {
    const size_t SIZE = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double start = omp_get_wtime();
        multiplier.multiplySequential();
        double seqTime = omp_get_wtime() - start;
        std::cout << "Sequential multiplication time: " << seqTime << " seconds\n";
        
        ParallelMatrixMultiplier parallelMultiplier(SIZE, SIZE, SIZE, SIZE);
        
        start = omp_get_wtime();
        parallelMultiplier.multiplyParallel();
        double parTime = omp_get_wtime() - start;
        std::cout << "Parallel multiplication time: " << parTime << " seconds\n";
        
        std::cout << "Speedup: " << seqTime / parTime << "x\n";
        
        double verificationError = parallelMultiplier.verifyMultiplication(multiplier);
        std::cout << "Verification error between sequential and parallel: " 
                  << verificationError << "\n";
        
        if (verificationError < 1e-10) {
            std::cout << "Multiplication verified successfully!\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}