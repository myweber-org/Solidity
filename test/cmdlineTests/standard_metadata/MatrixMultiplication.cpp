
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
    }

    void multiplyParallelOptimized(int chunkSize = 64) {
        #pragma omp parallel for schedule(dynamic, chunkSize)
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

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        if (result.size() != reference.size() || result[0].size() != reference[0].size()) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[0].size(); ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t rowsToPrint = std::min(matrix.size(), maxRows);
        size_t colsToPrint = std::min(matrix[0].size(), maxCols);
        
        for (size_t i = 0; i < rowsToPrint; ++i) {
            for (size_t j = 0; j < colsToPrint; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (colsToPrint < matrix[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (rowsToPrint < matrix.size()) {
            std::cout << "... (" << matrix.size() - rowsToPrint << " more rows)" << std::endl;
        }
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        sequentialResult = result;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        bool verification = verifyResult(sequentialResult);
        
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " 
                  << rowsB << "x" << colsB << std::endl;
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
        std::cout << "Verification: " << (verification ? "PASSED" : "FAILED") << std::endl;
        std::cout << "Available threads: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}