
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> resultMatrix;
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
        resultMatrix.resize(rowsA, std::vector<double>(colsB, 0.0));
    }

    void multiplySequential() {
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                resultMatrix[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                resultMatrix[i][j] = sum;
            }
        }
    }

    void multiplyParallelOptimized(int chunkSize = 64) {
        #pragma omp parallel for schedule(dynamic, chunkSize)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                resultMatrix[i][j] = sum;
            }
        }
    }

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        if (reference.size() != rowsA || reference[0].size() != colsB) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(resultMatrix[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(maxRows, matrix.size());
        size_t printCols = matrix.empty() ? 0 : std::min(maxCols, matrix[0].size());
        
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (printCols < matrix[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (printRows < matrix.size()) {
            std::cout << "..." << std::endl;
        }
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult;
        
        double start = omp_get_wtime();
        multiplySequential();
        sequentialResult = resultMatrix;
        double seqTime = omp_get_wtime() - start;
        
        start = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - start;
        
        bool verification = verifyResult(sequentialResult);
        
        std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
        std::cout << "Verification: " << (verification ? "PASSED" : "FAILED") << std::endl;
    }
};

int main() {
    const size_t N = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(N, N, N, N);
        
        std::cout << "Matrix Multiplication Benchmark (" << N << "x" << N << " matrices)" << std::endl;
        std::cout << "Number of threads: " << omp_get_max_threads() << std::endl;
        
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}