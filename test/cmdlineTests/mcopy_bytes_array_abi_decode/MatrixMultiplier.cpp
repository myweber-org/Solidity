
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

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        resultMatrix.resize(rowsA, std::vector<double>(colsB, 0.0));

        initializeRandomMatrices();
    }

    void initializeRandomMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }

        for (size_t i = 0; i < rowsB; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
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
        #pragma omp parallel for collapse(2) schedule(dynamic)
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

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double tolerance = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(resultMatrix[i][j] - reference[i][j]) > tolerance) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Result verification passed" << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(matrix.size(), maxRows);
        size_t printCols = matrix.empty() ? 0 : std::min(matrix[0].size(), maxCols);

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
        std::vector<std::vector<double>> sequentialResult = resultMatrix;

        double startTime = omp_get_wtime();
        multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        sequentialResult = resultMatrix;

        startTime = omp_get_wtime();
        multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;

        verifyResults(sequentialResult);

        std::cout << "Sequential execution time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel execution time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
    }
};

int main() {
    const size_t rowsA = 500;
    const size_t colsA = 500;
    const size_t rowsB = 500;
    const size_t colsB = 500;

    try {
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "Matrix A (first 5x5 elements):" << std::endl;
        multiplier.printMatrix(multiplier.matrixA);
        
        std::cout << "\nMatrix B (first 5x5 elements):" << std::endl;
        multiplier.printMatrix(multiplier.matrixB);
        
        std::cout << "\nBenchmarking matrix multiplication..." << std::endl;
        multiplier.benchmarkMultiplication();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}